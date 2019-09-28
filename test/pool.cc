
#include "gtest/gtest.h"

#include "threads/pool.hpp"

#include <chrono>

TEST(ThreadPool, BasicCaptures) {
  int data1 = 0;
  int data2 = 0;
  ThreadPool pool{1};

  auto future1 = pool.add_task([&] { data1 = 1; });
  auto future2 = pool.add_task([&] { data2 = 2; });

  auto status1 = future1.wait_for(std::chrono::seconds{10});
  EXPECT_EQ(std::future_status::ready, status1);
  EXPECT_EQ(1, data1);

  auto status2 = future2.wait_for(std::chrono::seconds{10});
  EXPECT_EQ(std::future_status::ready, status2);
  EXPECT_EQ(2, data2);

  std::this_thread::sleep_for(std::chrono::milliseconds{20});
  auto future3 = pool.add_task([&] { data1 = 3; });

  auto status3 = future3.wait_for(std::chrono::seconds{10});
  EXPECT_EQ(std::future_status::ready, status3);
  EXPECT_EQ(3, data1);
}

TEST(ThreadPool, FutureReturnsType) {
  ThreadPool pool{1};

  auto future1 = pool.add_task([] { return 100u; });
  auto future2 = pool.add_task([] { return "Hello"; });

  auto status1 = future1.wait_for(std::chrono::seconds{1});
  EXPECT_EQ(std::future_status::ready, status1);
  auto data1 = future1.get();
  static_assert(std::is_same<decltype(data1), unsigned>::value);
  EXPECT_EQ(100u, data1);

  auto status2 = future2.wait_for(std::chrono::seconds{1});
  EXPECT_EQ(std::future_status::ready, status2);
  auto data2 = future2.get();
  static_assert(std::is_same<decltype(data2), char const*>::value);
  EXPECT_EQ("Hello", data2);

  std::this_thread::sleep_for(std::chrono::milliseconds{20});
  auto future3 = pool.add_task([] {return 0.0;});

  auto status3 = future3.wait_for(std::chrono::seconds{1});
  EXPECT_EQ(std::future_status::ready, status3);
  auto data3 = future3.get();
  static_assert(std::is_same<decltype(data3), double>::value);
  EXPECT_EQ(0.0, data3);
}
