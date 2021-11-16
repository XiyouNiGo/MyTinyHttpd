#include "mytinyhttpd/net/timer.h"

namespace mytinyhttpd {

namespace net {

Atomic<int64_t> Timer::num_created_;

void Timer::restart(Timestamp now) {
  if (repeat_) {
    expiration_ = (now + Timestamp(interval_));
  } else {
    expiration_ = Timestamp::Invalid();
  }
}

}  // namespace net

}  // namespace mytinyhttpd
