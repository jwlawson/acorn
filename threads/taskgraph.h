/*
 * Copyright (c) 2020, John Lawson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef ACORN_THREADS_TASKGRAPH_H_
#define ACORN_THREADS_TASKGRAPH_H_

#include "container/slot_map.h"
#include "threads/shared_thread_pool.h"

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

namespace acorn {

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
      : pool_{n_threads}, mutex_{}, holding_queue_{} {}

  /**
   * Submit a task to be executed once its dependencies are fulfilled.
   *
   * @return A Task object containing both a task ID and a future for the task's
   * returned value. This task can be used as a dependency for further tasks.
   */
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

    auto base_task = TypedTask{
        FuncWrapper<Function>{std::forward<Function>(func), this, task_id}};
    auto future = base_task.get_future();

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

  /** Executor to handle executing tasks. */
  SharedThreadPool pool_;
  /** Mutex guarding multi-threaded access to the task queue. */
  Mutex mutex_;
  /** Queue of all pending, queued and running tasks, indexed by their ID. */
  TaskMap holding_queue_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace acorn

#endif  // ACORN_THREADS_TASKGRAPH_H_
