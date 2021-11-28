#include "mytinyhttpd/base/process_info.h"

#include <gtest/gtest.h>

namespace mytinyhttpd {

namespace process_info {

TEST(ProcessInfoTest, AllTest) {
  std::cout << "pid: " << pid() << std::endl;
  std::cout << "pid_string: " << pid_string() << std::endl;
  std::cout << "uid: " << uid() << std::endl;
  std::cout << "username: " << username() << std::endl;
  std::cout << "euid: " << euid() << std::endl;
  std::cout << "start_time: " << start_time().ToString() << std::endl;
  std::cout << "clock_ticks_per_second: " << clock_ticks_per_second()
            << std::endl;
  std::cout << "page_size: " << page_size() << std::endl;
  std::cout << "IsDebugBuild: " << IsDebugBuild() << std::endl;
  std::cout << "hostname: " << hostname() << std::endl;
  std::cout << "procname: " << procname() << std::endl;
  std::cout << "proc_status: " << proc_status() << std::endl;
  std::cout << "proc_stat: " << proc_stat() << std::endl;
  std::cout << "thread_stat: " << thread_stat() << std::endl;
  std::cout << "exe_path: " << exe_path() << std::endl;
  std::cout << "opened_files: " << opened_files() << std::endl;
  std::cout << "max_open_files: " << max_open_files() << std::endl;
  CpuTime t = cpu_time();
  std::cout << "cpu_time: " << t.user_seconds << t.system_seconds << std::endl;
  std::cout << "num_threads: " << num_threads() << std::endl;
}

}  // namespace process_info

}  // namespace mytinyhttpd