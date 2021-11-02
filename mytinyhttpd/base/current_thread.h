#ifndef MYTINYHTTPD_BASE_CURRENT_THREAD_H_
#define MYTINYHTTPD_BASE_CURRENT_THREAD_H_

#include <sys/types.h>

#include <cstdint>
#include <string>

#include "mytinyhttpd/base/thread.h"
#include "mytinyhttpd/utils/constants.h"

namespace mytinyhttpd {

namespace CurrentThread {

extern __thread pid_t cached_tid;
extern __thread char tid_string[sizeof(pid_t) * BIT_NUM_PER_BYTE];
extern __thread int8_t tid_string_length;
extern __thread const char* thread_name;

void CacheTid();

inline pid_t Tid() {
  if (unlikely(cached_tid == 0)) {
    CacheTid();
  }
  return cached_tid;
}

inline const char* TidString() { return tid_string; }

inline int8_t TidStringLength() { return tid_string_length; }

inline const char* Name() { return thread_name; }

inline bool IsMainThread() { return Tid() == getpid(); }

void SleepForUsec(int64_t usec);

}  // namespace CurrentThread

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_CURRENT_THREAD_H_