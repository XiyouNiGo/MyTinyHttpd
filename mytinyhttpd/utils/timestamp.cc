#include "mytinyhttpd/utils/timestamp.h"

#include <sys/time.h>

namespace mytinyhttpd {

static_assert(sizeof(Timestamp) == sizeof(int64_t),
              "Timestamp should be same size as int64_t");

std::string Timestamp::ToString() const {
  char buf[32] = {0};
  int64_t seconds = micro_seconds_since_epoch_ / kMicroSecondsPerSecond;
  int64_t microseconds = micro_seconds_since_epoch_ % kMicroSecondsPerSecond;
  ::snprintf(buf, sizeof(buf), "%ld.%06ld", seconds, microseconds);
  return buf;
}

std::string Timestamp::ToFormattedString(bool show_microseconds) const {
  char buf[64] = {0};
  time_t seconds =
      static_cast<time_t>(micro_seconds_since_epoch_ / kMicroSecondsPerSecond);
  struct tm tm_time;
  ::gmtime_r(&seconds, &tm_time);

  if (show_microseconds) {
    int microseconds =
        static_cast<int>(micro_seconds_since_epoch_ % kMicroSecondsPerSecond);
    ::snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
  } else {
    ::snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
  }
  return buf;
}

Timestamp Timestamp::Now() {
  struct timeval tv;
  // not a system call on the X64 architecture
  ::gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

}  // namespace mytinyhttpd