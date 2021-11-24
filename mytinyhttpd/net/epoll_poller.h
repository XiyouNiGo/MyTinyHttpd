#ifndef MYTINYHTTPD_NET_POLLER_EPOLLPOLLER_H_
#define MYTINYHTTPD_NET_POLLER_EPOLLPOLLER_H_

#include <sys/epoll.h>

#include <vector>

#include "mytinyhttpd/net/poller.h"

namespace mytinyhttpd {

namespace net {

class EPollPoller : public Poller {
 public:
  EPollPoller(EventLoop* loop);
  ~EPollPoller() override;

  Timestamp Poll(int timeoutMs, ChannelList* activeChannels) override;
  void UpdateChannel(Channel* channel) override;
  void RemoveChannel(Channel* channel) override;

 private:
  typedef std::vector<struct epoll_event> EventList;

  static const int kInitEventListSize = 16;

  static const char* OperationToString(int op);

  void FillActiveChannels(int numEvents, ChannelList* activeChannels) const;
  void Update(int operation, Channel* channel);

  int epollfd_;
  EventList events_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_POLLER_EPOLLPOLLER_H_
