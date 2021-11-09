#ifndef MYTINYHTTPD_BASE_PROCESSINFO_H_
#define MYTINYHTTPD_BASE_PROCESSINFO_H_

#include <sys/types.h>
#include <unistd.h>

#include <vector>

#include "mytinyhttpd/utils/slice.h"
#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

namespace process_info {

inline pid_t pid() { return getpid(); }
std::string pid_string();

inline uid_t uid() { return getuid(); }
std::string username();
inline uid_t euid() { return geteuid(); }

Timestamp start_time();
int clock_ticks_per_second();

int page_size();

bool IsDebugBuild();

std::string hostname();
std::string procname();
Slice procname(const std::string& stat);

std::string proc_status();

std::string proc_stat();

std::string thread_stat();

std::string exe_path();

int opened_files();
int max_open_files();

struct CpuTime {
  double user_seconds;
  double system_seconds;

  CpuTime() : user_seconds(0.0), system_seconds(0.0) {}

  double total() const { return user_seconds + system_seconds; }
};

struct CpuTime cpu_time();

int num_threads();
std::vector<pid_t> threads();

}  // namespace process_info

}  // namespace mytinyhttpd

#endif  // MYTINYHTTPD_BASE_PROCESSINFO_H_
