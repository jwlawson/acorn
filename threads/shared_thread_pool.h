/*
 * Copyright (c) 2020, John Lawson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
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
#ifndef ACORN_THREADS_SHARED_THREAD_POOL_H_
#define ACORN_THREADS_SHARED_THREAD_POOL_H_

#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"

namespace acorn {

/**
 * A basc thread pool executor with a single shared task queue.
 *
 * Workers will query the central shared queue for work once they have completed
 * a task.
 */
struct SharedThreadPool {
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
  /** Construct a SharedThreadPool with a set number of threads. */
  explicit SharedThreadPool(unsigned n_threads) {
    // Threads are not copyable, so have to create each separately
    thread_pool_.reserve(n_threads);
    for (unsigned count = 0; count < n_threads; ++count) {
      thread_pool_.emplace_back(&SharedThreadPool::worker_loop, this);
    }
  }

  SharedThreadPool() = delete;
  SharedThreadPool(SharedThreadPool const&) = delete;
  SharedThreadPool& operator=(SharedThreadPool const&) = delete;

  /**
   * Tear down the thread pool and wait for all currently queued tasks to be
   * completed.
   */
  ~SharedThreadPool() ABSL_LOCKS_EXCLUDED(mutex_) {
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

  /**
   * Add a task to be run on the ThreadPool.
   * @return A @c std::future which will be filled in with the return value of
   * the task once completed.
   */
  template <typename Function>
  auto add_task(Function&& func) -> std::future<decltype(func())> {
    using Return = decltype(func());
    using TypedTask = std::packaged_task<Return()>;

    auto new_task = TypedTask{std::move(func)};
    auto future = new_task.get_future();
    add_task(std::move(new_task));
    return future;
  }

  /** Add a packaged task to run n the SharedThreadPool. */
  template <typename ReturnType>
  void add_task(std::packaged_task<ReturnType()>&& task) {
    Lock lock{&mutex_};
    queue_.emplace(std::move(task));
  }

 private:
  /**
   * The main loop for each of the worker threads.
   *
   * Wait on the mutex until work is made available on the queue, then pull the
   * first task form the queue and execute that. An invalid task is used to
   * signal to the worker that the threadpool is shutting down, so the worker
   * should exit the loop.
   */
  void worker_loop() ABSL_LOCKS_EXCLUDED(mutex_) {
    while (true) {
      mutex_.Lock();
      mutex_.Await(absl::Condition{
          +[](TaskQueue* queue) { return !queue->empty(); }, &queue_});
      auto task = std::move(queue_.front());
      queue_.pop();
      mutex_.Unlock();
      if (!task.valid()) {
        // An invalid task signals the thread pool is shutting down.
        break;
      }
      task();
    }
  }

  /**
   * Pool of worker threads, used to execute queued tasks. Each worker thread
   * should invoke the ThreadPool::worker_loop.
   */
  ThreadContainer thread_pool_;
  /**
   * Mutex to guard the shared task queue, so that only one thread will execute
   * a given task, and queued tasks will not get lost. This means that not only
   * can this handle multiple workers it can also handle getting tasks from
   * multiple threads at once.
   */
  Mutex mutex_;
  /** The queue of tasks to be done. */
  TaskQueue queue_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace acorn

#endif  // ACORN_THREADS_SHARED_THREAD_POOL_H_
