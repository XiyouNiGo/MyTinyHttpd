#ifndef MYTINYHTTPD_UTILS_TIMEZONE_H_
#define MYTINYHTTPD_UTILS_TIMEZONE_H_

#include <time.h>

#include <memory>

#include "mytinyhttpd/utils/copyable.h"

namespace mytinyhttpd {

// using tzset/localtime_r to do time zone conversions can be problematic in
// multi-threaded environments
class TimeZone : public copyable {
 public:
  explicit TimeZone(const char* zonefile);
  TimeZone(int east_of_utc, const char* tzname);  // a fixed timezone
  TimeZone() = delete;

  bool Valid() const { return static_cast<bool>(rep_); }

  // localtime(3)
  struct tm ToLocalTime(time_t seconds_since_epoch) const;
  time_t FromLocalTime(const struct tm&) const;

  // gmtime(3)
  static struct tm ToUtcTime(time_t seconds_since_epoch, bool yday = false);
  // timegm(3)
  static time_t FromUtcTime(const struct tm&);
  static time_t FromUtcTime(int year, int month, int day, int hour, int minute,
                            int seconds);

  struct Rep;

 private:
  std::shared_ptr<Rep> rep_;
};

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_UTILS_TIMEZONE_H_