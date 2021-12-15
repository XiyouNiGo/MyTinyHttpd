#include <stdlib.h>

#include "mytinyhttpd/net/timing.h"
#include "mytinyhttpd/net/timing_list.h"
#include "mytinyhttpd/net/timing_wheel.h"

namespace mytinyhttpd {

namespace net {

Timing* Timing::NewDefaultTiming(int idle_seconds) {
  if (::getenv("MYTINYHTTPD_USE_TIMING_LIST")) {
    return new TimingList(idle_seconds);
  } else {
    return new TimingWheel(idle_seconds);
  }
}

}  // namespace net

}  // namespace mytinyhttpd