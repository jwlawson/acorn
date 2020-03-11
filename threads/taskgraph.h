

#include "threads/pool.h"
#include "container/slot_map.h"

#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

struct TaskGraph {
  struct InternalTask {
    /* The work required by this task. */
    std::packaged_task<void()> function;
    /* Number of tasks that must be completed before running this. */
    size_t n_dependencies;
    /* Tasks that depend on this task. */
    std::vector<size_t> dependees;
  };

  struct BaseTask {
    size_t const task_id;
  };
  template <typename ReturnType>
  struct Task : public BaseTask {
    std::future<ReturnType> future;

    Task(size_t id, std::future<ReturnType>&& fut)
        : BaseTask{id}, future{std::move(fut)} {}
  };

  template <typename Function>
  struct FuncWrapperBase {
    Function func_;
    TaskGraph* const graph_;
    size_t const task_id_;

    FuncWrapperBase(Function&& func, TaskGraph* graph, size_t id)
        : func_{std::move(func)}, graph_{graph}, task_id_{id} {}

    void task_complete() { graph_->task_complete(task_id_); }
    using ReturnType = decltype(func_());
  };

  template <
      typename Function,
      bool = std::is_same<decltype(std::declval<Function&>()()), void>::value>
  struct FuncWrapper : public FuncWrapperBase<Function> {
    using FuncWrapperBase<Function>::FuncWrapperBase;
    void operator()() {
      this->func_();
      this->task_complete();
    }
  };

  template <typename Function>
  struct FuncWrapper<Function, false> : public FuncWrapperBase<Function> {
    using FuncWrapperBase<Function>::FuncWrapperBase;
    using ReturnType = typename FuncWrapperBase<Function>::ReturnType;
    ReturnType operator()() {
      auto ret = this->func_();
      this->task_complete();
      return ret;
    }
  };

  using TaskMap = SlotMap<InternalTask>;
  using Mutex = absl::Mutex;
  using Lock = absl::MutexLock;

 public:
  TaskGraph(unsigned n_threads = 8)
      : pool_{n_threads}, mutex_{}, holding_queue_{}, last_id_{} {}

  template <typename Function, typename... Deps>
  auto submit(Function&& func, Deps const&... deps) -> Task<decltype(func())> {
    using Return = decltype(func());
    using TypedTask = std::packaged_task<Return()>;
    constexpr size_t NumDeps = sizeof...(Deps);

    size_t task_id;
    {
      Lock lock{&mutex_};
      task_id = holding_queue_.insert(InternalTask{{}, NumDeps, {}});
    }

    std::array<BaseTask, NumDeps> task_deps{deps...};
    for (auto&& dep : task_deps) {
      Lock lock{&mutex_};
      auto dep_id = dep.task_id;
      auto& dep_task = holding_queue_[dep_id];
      dep_task.dependees.push_back(task_id);
    }

    auto base_task =
        TypedTask{FuncWrapper<Function>{std::move(func), this, task_id}};
    auto future = base_task.get_future();

    InternalTask task;
    if (NumDeps == 0) {
      // This task has no dependencies, so forward directly to the executor. The
      // task stored in the queue will be needed to track tasks depending on
      // this but won't contain information about the task itself.
      pool_.add_task(std::move(base_task));
    } else {
      // The task has dependencies, so store the task in the holding queue
      // until they have been met.
      holding_queue_[task_id].function = std::move(base_task);
    }

    return {task_id, std::move(future)};
  }

 private:
  /** Executor to handle executing tasks. */
  ThreadPool pool_;
  /** Mutex guarding multi-threaded access to the task queue. */
  Mutex mutex_;
  /** Queue of all pending, queued and running tasks, indexed by their ID. */
  TaskMap holding_queue_ ABSL_GUARDED_BY(mutex_);
  /** The last task ID used in a task. */
  size_t last_id_;

  /** Get the next ID to use for a task. */
  size_t next_id() { return ++last_id_; }

  /**
   * Mark the task with provided ID as complete.
   *
   * Update any tasks that depend on the one just finished, and queue up any
   * that become unblocked.
   */
  void task_complete(int id) {
    Lock lock{&mutex_};
    auto& finished_task = holding_queue_[id];
    for (auto&& task_id : finished_task.dependees) {
      auto& task = holding_queue_[task_id];
      if (--task.n_dependencies == 0) {
        pool_.add_task(std::move(task.function));
      }
    }
    holding_queue_.erase(id);
  }
};
