
#include "gtest/gtest.h"

#include "threads/pool.h"

#include <chrono>

TEST(ThreadPool, BasicCaptures) {
  int data1 = 0;
  int data2 = 0;
  ThreadPool pool{1};

  auto future1 = pool.add_task([&] { data1 = 1; });
  auto future2 = pool.add_task([&] { data2 = 2; });

  auto status1 = future1.wait_for(std::chrono::seconds{10});
  ASSERT_EQ(std::future_status::ready, status1);
  EXPECT_EQ(1, data1);

  auto status2 = future2.wait_for(std::chrono::seconds{10});
  ASSERT_EQ(std::future_status::ready, status2);
  EXPECT_EQ(2, data2);

  std::this_thread::sleep_for(std::chrono::milliseconds{20});
  auto future3 = pool.add_task([&] { data1 = 3; });

  auto status3 = future3.wait_for(std::chrono::seconds{10});
  ASSERT_EQ(std::future_status::ready, status3);
  EXPECT_EQ(3, data1);
}

TEST(ThreadPool, FutureReturnsType) {
  ThreadPool pool{1};

  auto future1 = pool.add_task([] { return 100u; });
  auto future2 = pool.add_task([] { return "Hello"; });

  auto status1 = future1.wait_for(std::chrono::seconds{1});
  ASSERT_EQ(std::future_status::ready, status1);
  auto data1 = future1.get();
  static_assert(std::is_same<decltype(data1), unsigned>::value,
                "Wrong type deduced for Task 1 return type");
  EXPECT_EQ(100u, data1);

  auto status2 = future2.wait_for(std::chrono::seconds{1});
  ASSERT_EQ(std::future_status::ready, status2);
  auto data2 = future2.get();
  static_assert(std::is_same<decltype(data2), char const*>::value,
                "Wrong type deduced for Task 2 return type");
  EXPECT_EQ("Hello", data2);

  std::this_thread::sleep_for(std::chrono::milliseconds{20});
  auto future3 = pool.add_task([] { return 0.0; });

  auto status3 = future3.wait_for(std::chrono::seconds{1});
  ASSERT_EQ(std::future_status::ready, status3);
  auto data3 = future3.get();
  static_assert(std::is_same<decltype(data3), double>::value,
                "Wrong type deduced for Task 3 return type");
  EXPECT_EQ(0.0, data3);
}

TEST(ThreadPool, LotsOfSmallTasks) {
  ThreadPool pool{2};

  constexpr int data_size = 1024;
  std::vector<int> data(data_size, 0);
  std::vector<std::future<void>> futures(data_size);

  for (int count = 0; count < data_size; ++count) {
    futures[count] = pool.add_task([count, &data] { data[count] = count; });
  }

  for (int count = 0; count < data_size; ++count) {
    auto status = futures[count].wait_for(std::chrono::milliseconds{50});
    ASSERT_EQ(std::future_status::ready, status);
    (void)futures[count].get();
    EXPECT_EQ(count, data[count]);
  }
}

TEST(ThreadPool, SequentialLargerTasks) {
  ThreadPool pool{2};

  constexpr int n_tasks = 48;
  std::array<std::future<int>, n_tasks> futures{};

  for (int count = 0; count < n_tasks; ++count) {
    futures[count] = pool.add_task([count] {
      std::this_thread::sleep_for(std::chrono::milliseconds{10});
      return count;
    });
  }

  for (int count = 0; count < n_tasks; ++count) {
    auto status = futures[count].wait_for(std::chrono::milliseconds{500});
    ASSERT_EQ(std::future_status::ready, status);
    auto data = futures[count].get();
    EXPECT_EQ(count, data);
  }
}

TEST(ThreadPool, ParallelEnqueue) {
  ThreadPool pool{2};

  auto enqueue_and_test = [&pool] {
    constexpr int n_tasks = 48;
    std::array<std::future<int>, n_tasks> futures{};

    for (int count = 0; count < n_tasks; ++count) {
      futures[count] = pool.add_task([count] { return count; });
    }

    for (int count = 0; count < n_tasks; ++count) {
      auto status = futures[count].wait_for(std::chrono::milliseconds{500});
      ASSERT_EQ(std::future_status::ready, status);
      auto data = futures[count].get();
      EXPECT_EQ(count, data);
    }
  };

  std::thread thread1{enqueue_and_test};
  std::thread thread2{enqueue_and_test};
  std::thread thread3{enqueue_and_test};
  std::thread thread4{enqueue_and_test};
  std::thread thread5{enqueue_and_test};

  thread1.join();
  thread2.join();
  thread3.join();
  thread4.join();
  thread5.join();
}

TEST(ThreadPool, StdFunctionAliveOutOfScope) {
  ThreadPool pool{1};

  auto enqueue = [&pool](int retval) {
    std::function<int()> func1 = [retval] { return retval; };
    return pool.add_task(func1);
  };
  auto future1 = enqueue(1);
  auto future2 = enqueue(2);

  auto status1 = future1.wait_for(std::chrono::seconds{1});
  ASSERT_EQ(std::future_status::ready, status1);
  auto data1 = future1.get();
  EXPECT_EQ(1, data1);

  auto status2 = future2.wait_for(std::chrono::seconds{1});
  ASSERT_EQ(std::future_status::ready, status2);
  auto data2 = future2.get();
  EXPECT_EQ(2, data2);
}

TEST(ThreadPool, PoolDestructorWaits) {
  std::future<int> future;
  {
    ThreadPool pool{1};
    future = pool.add_task([] {
      std::this_thread::sleep_for(std::chrono::milliseconds{25});
      return 10;
    });
  }
  auto status = future.wait_for(std::chrono::milliseconds{0});
  ASSERT_EQ(std::future_status::ready, status);
  auto data = future.get();
  EXPECT_EQ(10, data);
}
