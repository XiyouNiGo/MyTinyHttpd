#ifndef MYTINYHTTPD_BASE_CURRENT_THREAD_H_
#define MYTINYHTTPD_BASE_CURRENT_THREAD_H_

#include <sys/types.h>

#include <cstdint>
#include <string>

#include "mytinyhttpd/base/thread.h"
#include "mytinyhttpd/utils/constants.h"

namespace mytinyhttpd {

namespace CurrentThread {

extern __thread pid_t t_cached_tid;
extern __thread char t_tid_string[sizeof(pid_t) * BIT_NUM_PER_BYTE];
extern __thread int8_t t_tid_string_length;
extern __thread const char* t_thread_name;

void CacheTid();

inline pid_t tid() {
  if (unlikely(t_cached_tid == 0)) {
    CacheTid();
  }
  return t_cached_tid;
}

inline const char* tid_string() { return t_tid_string; }

inline int8_t tid_string_length() { return t_tid_string_length; }

inline const char* name() { return t_thread_name; }

inline bool IsMainThread() { return tid() == getpid(); }

void SleepForUsec(int64_t usec);

}  // namespace CurrentThread

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_CURRENT_THREAD_H_