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

#include "threads/logger.h"

#include <future>

TEST(Logger, BasicLogOutput) {
  std::stringstream ss;
  acorn::StreamWriter writer{ss};

  acorn::LogEntry{&writer, __FILE__, __LINE__} << "hello"
                                               << " "
                                               << "world";

  auto out = ss.str();
  EXPECT_THAT(out, ::testing::HasSubstr("hello"));
  EXPECT_THAT(out, ::testing::HasSubstr("world"));
}

TEST(Logger, NoIntermingleWithThreads) {
  std::stringstream ss;
  acorn::StreamWriter writer{ss};

  auto append_to_log = [&](char const* first, char const* second) {
    acorn::LogEntry{&writer, __FILE__, __LINE__} << first << " " << second;
  };

  auto f1 = std::async(append_to_log, "hello", "world");
  auto f2 = std::async(append_to_log, "one", "two");
  auto f3 = std::async(append_to_log, "three", "four");
  f1.wait();
  f2.wait();
  f3.wait();

  auto out = ss.str();
  EXPECT_THAT(out, ::testing::HasSubstr("hello world"));
  EXPECT_THAT(out, ::testing::HasSubstr("one two"));
  EXPECT_THAT(out, ::testing::HasSubstr("three four"));
}

TEST(StdoutLogger, BasicPrint) {
  LOG() << "Hello"
        << " "
        << "world";
}

TEST(BufferedWriter, LogPrintedOnDestruction) {
  std::stringstream ss;
  acorn::StreamWriter base_writer{ss};
  {
    acorn::BufferedWriter<> writer{&base_writer};

    acorn::LogEntry{&writer, __FILE__, __LINE__} << "hello"
                                                 << " "
                                                 << "world";
    acorn::LogEntry{&writer, __FILE__, __LINE__} << "one two";
    acorn::LogEntry{&writer, __FILE__, __LINE__} << "three four";
  }
  auto out = ss.str();
  EXPECT_THAT(out, ::testing::HasSubstr("hello"));
  EXPECT_THAT(out, ::testing::HasSubstr("world"));
  EXPECT_THAT(out, ::testing::HasSubstr("one two"));
  EXPECT_THAT(out, ::testing::HasSubstr("three four"));
}
