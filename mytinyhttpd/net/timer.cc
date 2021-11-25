#include "mytinyhttpd/net/timer.h"

#include "mytinyhttpd/utils/constants.h"

namespace mytinyhttpd {

namespace net {

Atomic<int64_t> Timer::num_created_;

void Timer::Restart(Timestamp now) {
  if (likely(is_repeat_)) {
    expiration_ = (now + Timestamp(interval_));
  } else {
    expiration_ = Timestamp::Invalid();
  }
}

}  // namespace net

}  // namespace mytinyhttpd
