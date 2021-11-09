#include "mytinyhttpd/base/process_info.h"

#include <assert.h>
#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <unistd.h>

#include <algorithm>

#include "mytinyhttpd/base/current_thread.h"
#include "mytinyhttpd/utils/file.h"

namespace mytinyhttpd {

namespace detail {

__thread int t_num_opened_files = 0;

int FdDirFilter(const struct dirent* d) {
  if (isdigit(d->d_name[0])) {
    ++t_num_opened_files;
  }
  return 0;
}

__thread std::vector<pid_t>* t_pids = NULL;

int TaskDirFilter(const struct dirent* d) {
  if (isdigit(d->d_name[0])) {
    t_pids->push_back(atoi(d->d_name));
  }
  return 0;
}

int ScanDir(const char* dirpath, int (*filter)(const struct dirent*)) {
  struct dirent** namelist = NULL;
  int result = scandir(dirpath, &namelist, filter, alphasort);
  assert(namelist == NULL);
  return result;
}

Timestamp g_start_time = Timestamp::Now();

int g_clock_ticks = static_cast<int>(sysconf(_SC_CLK_TCK));
int g_page_size = static_cast<int>(sysconf(_SC_PAGE_SIZE));

}  // namespace detail

std::string process_info::pid_string() {
  char buf[32];
  snprintf(buf, sizeof(buf), "%d", pid());
  return buf;
}

std::string process_info::username() {
  struct passwd pwd;
  struct passwd* result = NULL;
  char buf[8192];
  const char* name = "unknownuser";

  getpwuid_r(uid(), &pwd, buf, sizeof(buf), &result);
  if (result) {
    name = pwd.pw_name;
  }
  return name;
}

Timestamp process_info::start_time() { return detail::g_start_time; }

int process_info::clock_ticks_per_second() { return detail::g_clock_ticks; }

int process_info::page_size() { return detail::g_page_size; }

bool process_info::IsDebugBuild() {
#ifdef NDEBUG
  return false;
#else
  return true;
#endif
}

std::string process_info::hostname() {
  char buf[256];
  if (gethostname(buf, sizeof(buf)) == 0) {
    buf[sizeof(buf) - 1] = '\0';
    return buf;
  } else {
    return "unknownhost";
  }
}

std::string process_info::procname() {
  return procname(proc_stat()).ToString();
}

Slice process_info::procname(const std::string& stat) {
  size_t lp = stat.find('(');
  size_t rp = stat.rfind(')');
  if (lp != std::string::npos && rp != std::string::npos && lp < rp) {
    return Slice(stat.data() + lp + 1, static_cast<int>(rp - lp - 1));
  }
  return Slice();
}

std::string process_info::proc_status() {
  std::string result;
  ReadFile("/proc/self/status", 65536, result);
  return result;
}

std::string process_info::proc_stat() {
  std::string result;
  ReadFile("/proc/self/stat", 65536, result);
  return result;
}

std::string process_info::thread_stat() {
  char buf[64];
  snprintf(buf, sizeof(buf), "/proc/self/task/%d/stat", CurrentThread::tid());
  std::string result;
  ReadFile(buf, 65536, result);
  return result;
}

std::string process_info::exe_path() {
  std::string result;
  char buf[1024];
  ssize_t n = ::readlink("/proc/self/exe", buf, sizeof(buf));
  if (n > 0) {
    result.assign(buf, n);
  }
  return result;
}

int process_info::opened_files() {
  detail::t_num_opened_files = 0;
  detail::ScanDir("/proc/self/fd", detail::FdDirFilter);
  return detail::t_num_opened_files;
}

int process_info::max_open_files() {
  struct rlimit rl;
  return getrlimit(RLIMIT_NOFILE, &rl) ? opened_files()
                                       : static_cast<int>(rl.rlim_cur);
}

process_info::CpuTime process_info::cpu_time() {
  CpuTime t;
  struct tms tms;
  if (times(&tms) >= 0) {
    const double hz = static_cast<double>(clock_ticks_per_second());
    t.user_seconds = static_cast<double>(tms.tms_utime) / hz;
    t.system_seconds = static_cast<double>(tms.tms_stime) / hz;
  }
  return t;
}

int process_info::num_threads() {
  int result = 0;
  std::string status = proc_status();
  size_t pos = status.find("Threads:");
  if (pos != std::string::npos) {
    result = atoi(status.c_str() + pos + 8);
  }
  return result;
}

std::vector<pid_t> process_info::threads() {
  std::vector<pid_t> result;
  detail::t_pids = &result;
  detail::ScanDir("/proc/self/task", detail::TaskDirFilter);
  detail::t_pids = NULL;
  std::sort(result.begin(), result.end());
  return result;
}

}  // namespace mytinyhttpd
