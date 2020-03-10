
#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"

#include <iostream>
#include <ostream>
#include <sstream>

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

#define LOG() LogEntry(stdout_writer(), __FILE__, __LINE__)
