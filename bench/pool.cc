
#include "threads/pool.h"

#include "benchmark/benchmark.h"

#include <chrono>
#include <future>
#include <numeric>
#include <vector>

#include <iostream>

static void ManySmallTasks(::benchmark::State& state) {
  auto n_threads = std::thread::hardware_concurrency();
  auto n_tasks = state.range(0);
  ThreadPool pool{n_threads};
  std::vector<std::future<void>> futures(n_tasks);

  for ([[maybe_unused]] auto&& _ : state) {
    for (int i = 0; i < n_tasks; ++i) {
      auto future = pool.add_task(
          [] { std::this_thread::sleep_for(std::chrono::nanoseconds{100}); });
      futures[i] = std::move(future);
    }
    for (auto&& future : futures) {
      future.wait();
    }
  }
}
BENCHMARK(ManySmallTasks)->Ranges({{1 << 8, 1 << 15}});

static void FewLargeTasks(::benchmark::State& state) {
  auto n_threads = std::thread::hardware_concurrency();
  auto n_tasks = state.range(0);
  auto n_values = state.range(1);
  ThreadPool pool{n_threads};
  std::vector<std::future<float>> futures(n_tasks);
  std::vector<float> data(n_values);

  for ([[maybe_unused]] auto&& _ : state) {
    for (int i = 0; i < n_tasks; ++i) {
      auto future = pool.add_task([&] {
        auto val = std::accumulate(begin(data), end(data), 0.1f);
        return val;
      });
      futures[i] = std::move(future);
    }
    for (auto& future : futures) {
      auto val = future.get();
      ::benchmark::DoNotOptimize(val);
    }
  }
}
BENCHMARK(FewLargeTasks)->Ranges({{4, 32}, {1 << 10, 1 << 16}});
