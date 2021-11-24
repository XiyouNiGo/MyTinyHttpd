#ifndef MYTINYHTTPD_NET_POLLER_POLLPOLLER_H_
#define MYTINYHTTPD_NET_POLLER_POLLPOLLER_H_

#include <poll.h>

#include <vector>

#include "mytinyhttpd/net/poller.h"

namespace mytinyhttpd {

namespace net {

class PollPoller : public Poller {
 public:
  PollPoller(EventLoop* loop);
  ~PollPoller() override;

  Timestamp Poll(int timeout_ms, ChannelList* active_channels) override;
  void UpdateChannel(Channel* channel) override;
  void RemoveChannel(Channel* channel) override;

 private:
  void FillActiveChannels(int num_events, ChannelList* active_channels) const;

  typedef std::vector<struct pollfd> PollFdList;
  PollFdList pollfds_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_POLLER_POLLPOLLER_H_
