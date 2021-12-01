#ifndef MYTINYHTTPD_NET_TIMER_ID_H_
#define MYTINYHTTPD_NET_TIMER_ID_H_

#include <cstdint>

#include "mytinyhttpd/net/timer.h"
#include "mytinyhttpd/utils/copyable.h"

namespace mytinyhttpd {

namespace net {

class TimerId : public copyable {
 public:
  TimerId() : timer_(nullptr), sequence_(0) {}

  TimerId(Timer* timer, int64_t seq) : timer_(timer), sequence_(seq) {}

  friend class TimerQueue;

 private:
  Timer* timer_;
  int64_t sequence_;  // distinguish between two Timestamp objects with
                      // the same address
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_TIMER_ID_H_
