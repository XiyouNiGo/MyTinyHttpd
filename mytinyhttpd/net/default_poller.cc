#include <stdlib.h>

#include "mytinyhttpd/net/epoll_poller.h"
#include "mytinyhttpd/net/poll_poller.h"
#include "mytinyhttpd/net/poller.h"

namespace mytinyhttpd {

namespace net {

Poller* Poller::NewDefaultPoller(EventLoop* loop) {
  if (::getenv("MYTINYHTTPD_USE_POLL")) {
    return new PollPoller(loop);
  } else {
    return new EPollPoller(loop);
  }
}

}  // namespace net

}  // namespace mytinyhttpd