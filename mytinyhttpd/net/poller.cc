#include "mytinyhttpd/net/poller.h"

#include "mytinyhttpd/net/channel.h"

namespace mytinyhttpd {

namespace net {

Poller::Poller(EventLoop* loop) : loop_(loop) {}

Poller::~Poller() = default;

bool Poller::HasChannel(Channel* channel) const {
  AssertInLoopThread();
  ChannelMap::const_iterator it = channels_.find(channel->fd());
  return it != channels_.end() && it->second == channel;
}

}  // namespace net

}  // namespace mytinyhttpd