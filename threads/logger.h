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
#ifndef ACORN_THREADS_LOGGER_H_
#define ACORN_THREADS_LOGGER_H_

#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"

#include <iostream>
#include <ostream>
#include <sstream>

namespace acorn {

struct Writer {
  virtual ~Writer() = default;
  virtual void write(absl::string_view str) = 0;
};

struct LogEntry {
  LogEntry(Writer* writer, char const* filename, int line_no)
      : message_builder_{}, writer_{writer} {
    add_prefix_to_message(filename, line_no);
  }
  ~LogEntry() {
    message_builder_ << "\n";
    writer_->write(message_builder_.str());
  }

  template <typename T>
  LogEntry& operator<<(T const& t) {
    message_builder_ << t;
    return *this;
  }

 private:
  std::ostringstream message_builder_;
  Writer* writer_;

  void add_prefix_to_message(char const* filename, int line_no) {
    auto time = absl::Now();
    auto timezone = absl::LocalTimeZone();
    message_builder_ << "[" << absl::FormatTime(time, timezone) << " "
                     << filename << ":" << line_no << "] ";
  }
};

struct StreamWriter final : Writer {
 private:
  using Mutex = absl::Mutex;
  using Lock = absl::MutexLock;

 public:
  StreamWriter(std::ostream& os) : os_{os} {}

  void write(absl::string_view str) override {
    Lock lock{&mutex_};
    os_ << str;
  }

 private:
  Mutex mutex_;
  std::ostream& os_ ABSL_GUARDED_BY(mutex_);
};

template <int BufferSize = 16>
struct BufferedWriter final : Writer {
 private:
  using Mutex = absl::Mutex;
  using Lock = absl::MutexLock;

 public:
  BufferedWriter(Writer* writer) : base_writer_{writer} {}
  ~BufferedWriter() override {
    Lock lock{&mutex_};
    write_buffer_to_base();
  }

  void write(absl::string_view str) override {
    buffer_ << str;
    if (++buffered_logs_ > BufferSize) {
      Lock lock{&mutex_};
      write_buffer_to_base();
    }
  }

 private:
  Mutex mutex_{};
  Writer* base_writer_ ABSL_GUARDED_BY(mutex_);
  std::ostringstream buffer_{};
  int buffered_logs_{0};

  void write_buffer_to_base() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mutex_) {
    for (int i = 0; i < buffered_logs_; ++i) {
      base_writer_->write(buffer_.str());
      buffer_.clear();
    }
    buffered_logs_ = 0;
  }
};

Writer* stdout_writer() {
  static StreamWriter writer{std::cout};
  return &writer;
}

}  // namespace acorn

#define LOG() acorn::LogEntry(acorn::stdout_writer(), __FILE__, __LINE__)

#endif  // ACORN_THREADS_LOGGER_H_
