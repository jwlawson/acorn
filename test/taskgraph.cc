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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "acorn/threads/taskgraph.h"

#include <chrono>

TEST(TaskGraph, NoDepsSingleExecutor) {
  acorn::TaskGraph graph{1};

  int count = 0;
  graph.submit([&] { count++; });
  graph.submit([&] { count++; });
  graph.submit([&] { count++; });
  auto last = graph.submit([&] { count++; });

  last.future.wait_for(std::chrono::milliseconds{50});

  EXPECT_EQ(count, 4);
}

TEST(TaskGraph, WithDeps) {
  acorn::TaskGraph graph;

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
  acorn::TaskGraph graph;

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
