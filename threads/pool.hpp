
#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

struct ThreadPool {
 private:
  using Mutex = std::mutex;
  using Lock = std::unique_lock<Mutex>;
  using ConditionVar = std::condition_variable;

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
      auto lock = Lock{mutex_};
      shared_.is_active_ = false;
    }
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
      auto lock = Lock{mutex_};
      shared_.queue_.emplace(std::move(new_task));
    }
    worker_notifier_.notify_one();

    return future;
  }

 private:
  ThreadContainer thread_pool_;
  ConditionVar worker_notifier_;

  Mutex mutex_;

  struct SharedData {
    TaskQueue queue_;
    bool is_active_{true};
  };
  SharedData shared_;

  void worker_loop() {
    auto lock = Lock{mutex_};
    while (true) {
      worker_notifier_.wait(lock, [this] {
        return !shared_.is_active_ || !shared_.queue_.empty();
      });
      if (!shared_.is_active_) {
        return;
      }
      auto task = std::move(shared_.queue_.front());
      shared_.queue_.pop();
      lock.unlock();
      task();
      lock.lock();
    }
  }
};
