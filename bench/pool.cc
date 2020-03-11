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

#include "acorn/macros.h"
#include "acorn/threads/shared_thread_pool.h"

#include "benchmark/benchmark.h"

#include <chrono>
#include <future>
#include <numeric>
#include <vector>

static void ManySmallTasks(::benchmark::State& state) {
  auto n_threads = std::thread::hardware_concurrency();
  auto n_tasks = state.range(0);
  acorn::SharedThreadPool pool{n_threads};
  std::vector<std::future<void>> futures(n_tasks);

  for (ACORN_MAYBE_UNUSED auto&& _ : state) {
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
BENCHMARK(ManySmallTasks)->Ranges({{1 << 8, 1 << 14}})->UseRealTime();

static void FewLargeTasks(::benchmark::State& state) {
  auto n_threads = std::thread::hardware_concurrency();
  auto n_tasks = state.range(0);
  auto n_values = state.range(1);
  acorn::SharedThreadPool pool{n_threads};
  std::vector<std::future<float>> futures(n_tasks);
  std::vector<float> data(n_values);

  for (ACORN_MAYBE_UNUSED auto&& _ : state) {
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
BENCHMARK(FewLargeTasks)->Ranges({{4, 16}, {1 << 10, 1 << 15}})->UseRealTime();
