#include "threads/pool.h"

#include <chrono>

int main() {
  int data1 = 0;
  int data2 = 0;
  ThreadPool pool{1};

  auto future1 = pool.add_task([&] { data1 = 1; });
  auto future2 = pool.add_task([&] { data2 = 2; });

  auto status1 = future1.wait_for(std::chrono::seconds{10});
  if (std::future_status::ready != status1) {
    return -1;
  }

  auto status2 = future2.wait_for(std::chrono::seconds{10});
  if (std::future_status::ready != status2) {
    return -1;
  }

  return data1 + data2 != 3;
}

