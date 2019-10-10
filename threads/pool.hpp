
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
  // Construct a ThreadPool with a set number of threads.
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

  // Tear down the ThreadPool and wait for all currently queued tasks to be
  // completed.
  ~ThreadPool() ABSL_LOCKS_EXCLUDED(mutex_) {
    {
      Lock lock{&mutex_};
      for (unsigned count = 0; count < thread_pool_.size(); ++count) {
        // Add an invalid task to end of queue to signal shutdown. This ensures
        // all queued tasks get finished.
        queue_.emplace(Task{});
      }
    }
    for (auto& thread : thread_pool_) {
      thread.join();
    }
  }

  // Add a task to be run on the ThreadPool.
  // Returns a std::future which will be filled in with the return value of the
  // task once completed.
  template <typename Function>
  auto add_task(Function&& func) -> std::future<decltype(func())> {
    using Return = decltype(func());
    using TypedTask = std::packaged_task<Return()>;

    auto new_task = TypedTask{func};
    auto future = new_task.get_future();
    {
      Lock lock{&mutex_};
      queue_.emplace(std::move(new_task));
    }
    return future;
  }

 private:
  // The main loop for each of the worker threads.
  // Wait on the mutex until work is made available on the queue, then pull the
  // first task form the queue and execute that. An invalid task is used to
  // signal to the worker that the threadpool is shutting down, so the worker
  // should exit the loop.
  void worker_loop() ABSL_LOCKS_EXCLUDED(mutex_) {
    while (true) {
      mutex_.Lock();
      mutex_.Await(absl::Condition{
          +[](TaskQueue* queue) { return !queue->empty(); }, &queue_});
      auto task = std::move(queue_.front());
      queue_.pop();
      mutex_.Unlock();
      if (!task.valid()) {
        // Invalid task signals the thread pool is shutting down.
        break;
      }
      task();
    }
  }

  // Pool of worker threads, used to execute queued tasks. Each worker thread
  // should invoke the ThreadPool::worker_loop.
  ThreadContainer thread_pool_;
  // Mutex to guard the shared task queue, so that only one thread will execute
  // a given task, and queued tasks will not get lost. This means that not only
  // can this handle multiple workers it can also handle getting tasks from
  // multiple threads at once.
  Mutex mutex_;
  // The queue of tasks to be done. Access to this must be guarded by mutex_.
  TaskQueue queue_ ABSL_GUARDED_BY(mutex_);
};
