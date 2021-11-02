#include "mytinyhttpd/base/current_thread.h"

#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

namespace CurrentThread {

__thread pid_t cached_tid;
__thread char tid_string[sizeof(pid_t) * BIT_NUM_PER_BYTE];
__thread int8_t tid_string_length;
__thread const char* thread_name;

void CacheTid() {
  if (cached_tid == 0) {
    cached_tid = detail::gettid();
    tid_string_length = static_cast<int8_t>(
        snprintf(tid_string, sizeof(tid_string), "%5d ", cached_tid));
  }
}

// usec -> micro second
void SleepForUsec(int64_t usec) {
  struct timespec ts = {0, 0};
  ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond *
                                 Timestamp::kNanoSecondsPerMicroSecond);
  // nanosleep is thread-safe
  nanosleep(&ts, NULL);
}

}  // namespace CurrentThread

}  // namespace mytinyhttpd