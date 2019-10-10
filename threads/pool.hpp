
#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"

struct ThreadPool {
 private:
  using Mutex = absl::Mutex;
  using Lock = absl::MutexLock;

  using Task = std::packaged_task<void()>;
  using TaskQueueAllocator = std::allocator<Task>;
  using TaskQueueContainer = std::deque<Task, TaskQueueAllocator>;
  using TaskQueue = std::queue<Task, TaskQueueContainer>;

  using Thread = std::thread;
  using ThreadContainer = std::vector<Thread>;

 public:
  explicit ThreadPool(unsigned n_threads) {
    // Threads are not copyable, so have to create each separately
    thread_pool_.reserve(n_threads);
    for (unsigned count = 0; count < n_threads; ++count) {
      thread_pool_.emplace_back(&ThreadPool::worker_loop, this);
    }
  }

  ThreadPool() = delete;
  ThreadPool(ThreadPool const&) = delete;
  ThreadPool& operator=(ThreadPool const&) = delete;

  ~ThreadPool() {
    {
      Lock lock{&mutex_};
      shared_.is_active_ = false;
    }

    for (auto& thread : thread_pool_) {
      thread.join();
    }
  }

  template <typename Function>
  auto add_task(Function&& func) -> std::future<decltype(func())> {
    using Return = decltype(func());
    using TypedTask = std::packaged_task<Return()>;

    auto new_task = TypedTask{func};
    auto future = new_task.get_future();

    {
      Lock lock{&mutex_};
      shared_.queue_.emplace(std::move(new_task));
    }

    return future;
  }

 private:
  ThreadContainer thread_pool_;

  Mutex mutex_;

  struct SharedData {
    TaskQueue queue_;
    bool is_active_{true};
  };
  SharedData shared_ ABSL_GUARDED_BY(mutex_);

  bool worker_should_wake() ABSL_SHARED_LOCKS_REQUIRED(mutex_) {
    return !shared_.is_active_ || !shared_.queue_.empty();
  }

  void worker_loop() {
    while (true) {
      mutex_.Lock();
      mutex_.Await(absl::Condition(this, &ThreadPool::worker_should_wake));
      if (!shared_.is_active_) {
        mutex_.Unlock();
        return;
      }
      auto task = std::move(shared_.queue_.front());
      shared_.queue_.pop();
      mutex_.Unlock();
      task();
    }
  }
};
