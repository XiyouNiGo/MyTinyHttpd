#include "mytinyhttpd/base/current_thread.h"

#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

namespace CurrentThread {

__thread pid_t t_cached_tid;
__thread char t_tid_string[sizeof(pid_t) * BIT_NUM_PER_BYTE];
__thread int8_t t_tid_string_length;
__thread const char* t_thread_name;

void CacheTid() {
  if (t_cached_tid == 0) {
    t_cached_tid = detail::gettid();
    t_tid_string_length = static_cast<int8_t>(
        snprintf(t_tid_string, sizeof(t_tid_string), "%5d ", t_cached_tid));
  }
}

pid_t tid() {
  if (unlikely(t_cached_tid == 0)) {
    CacheTid();
  }
  return t_cached_tid;
}

// usec -> micro second
void SleepForUsec(int64_t usec) {
  struct timespec ts = {0, 0};
  ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond *
                                 Timestamp::kNanoSecondsPerMicroSecond);
  // nanosleep is thread-safe
  nanosleep(&ts, nullptr);
}

}  // namespace CurrentThread

}  // namespace mytinyhttpd