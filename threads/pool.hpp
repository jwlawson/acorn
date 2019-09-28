
#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

//#include <iostream>

#define LOG(X) std::cerr

struct ThreadPool {
 private:
  using Mutex = std::mutex;
  using Lock = std::unique_lock<Mutex>;
  using Task = std::packaged_task<void()>;
  using TaskQueue = std::queue<Task>;
  using Thread = std::thread;
  using ThreadContainer = std::vector<Thread>;
  using ConditionVar = std::condition_variable;

 public:
  ThreadPool(int n_threads) {
    // Threads are not copyable, so have to create each separately
    thread_pool_.reserve(n_threads);
    for (int count = 0; count < n_threads; ++count) {
      thread_pool_.emplace_back(&ThreadPool::worker_loop, this);
    }
  }
  ~ThreadPool() {
    active_ = false;
    worker_notifier_.notify_all();

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
      auto lock = Lock{queue_mutex_};
      task_queue_.emplace(std::move(new_task));
    }
    worker_notifier_.notify_one();

    return future;
  }

 private:
  ThreadContainer thread_pool_;
  TaskQueue task_queue_;
  Mutex queue_mutex_;
  ConditionVar worker_notifier_;
  std::atomic<bool> active_{true};

  void wait_for_new_task() {
    auto lock = Lock{queue_mutex_};
    worker_notifier_.wait(lock,
                          [this] { return !active_ || !task_queue_.empty(); });
  }

  void worker_loop() {
    while (active_) {
      {
        auto lock = Lock{queue_mutex_};
        if (!task_queue_.empty()) {
          auto task = std::move(task_queue_.front());
          task_queue_.pop();
          lock.unlock();
          task();
        } else {
          //LOG(WARNING) << "Thought there was work, but there wasn't\n";
        }
      }
      wait_for_new_task();
    }
  }
};
