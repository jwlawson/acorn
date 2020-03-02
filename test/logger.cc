
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "threads/logger.h"

#include <future>

TEST(Logger, BasicLogOutput) {
  std::stringstream ss;
  StreamWriter writer{ss};

  LogEntry{&writer, __FILE__, __LINE__} << "hello"
                                        << " "
                                        << "world";

  auto out = ss.str();
  EXPECT_THAT(out, ::testing::HasSubstr("hello"));
  EXPECT_THAT(out, ::testing::HasSubstr("world"));
}

TEST(Logger, NoIntermingleWithThreads) {
  std::stringstream ss;
  StreamWriter writer{ss};

  auto append_to_log = [&](char const* first, char const* second) {
    LogEntry{&writer, __FILE__, __LINE__} << first << " " << second;
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
  StreamWriter base_writer{ss};
  {
    BufferedWriter<> writer{&base_writer};

    LogEntry{&writer, __FILE__, __LINE__} << "hello"
                                          << " "
                                          << "world";
    LogEntry{&writer, __FILE__, __LINE__} << "one two";
    LogEntry{&writer, __FILE__, __LINE__} << "three four";
  }
  auto out = ss.str();
  EXPECT_THAT(out, ::testing::HasSubstr("hello"));
  EXPECT_THAT(out, ::testing::HasSubstr("world"));
  EXPECT_THAT(out, ::testing::HasSubstr("one two"));
  EXPECT_THAT(out, ::testing::HasSubstr("three four"));
}
