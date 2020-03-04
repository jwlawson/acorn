
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "threads/taskgraph.h"

#include <chrono>

TEST(TaskGraph, NoDepsSingleExecutor) {
  TaskGraph graph{1};

  int count = 0;
  graph.submit([&] { count++; });
  graph.submit([&] { count++; });
  graph.submit([&] { count++; });
  auto last = graph.submit([&] { count++; });

  last.future.wait_for(std::chrono::milliseconds{50});

  EXPECT_EQ(count, 4);
}

TEST(TaskGraph, WithDeps) {
  TaskGraph graph;

  int count = 0;

  auto a = graph.submit([&] {
    std::this_thread::sleep_for(std::chrono::milliseconds{20});
    count++;
  });
  auto b = graph.submit([&] { count = -count; }, a);
  b.future.wait_for(std::chrono::milliseconds{100});

  EXPECT_EQ(count, -1);
}

TEST(TaskGraph, TransientDeps) {
  TaskGraph graph;

  int count = 0;
  auto a = graph.submit([&] {
    std::this_thread::sleep_for(std::chrono::milliseconds{50});
    count++;
  });
  auto b = graph.submit(
      [&] {
        std::this_thread::sleep_for(std::chrono::milliseconds{30});
        count = -count;
      },
      a);
  auto c = graph.submit(
      [&] {
        std::this_thread::sleep_for(std::chrono::milliseconds{30});
        count += 10;
      },
      b);
  c.future.wait_for(std::chrono::milliseconds{200});

  EXPECT_EQ(count, 9);
}
