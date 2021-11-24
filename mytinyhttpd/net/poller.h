#ifndef MYTINYHTTPD_NET_POLLER_H_
#define MYTINYHTTPD_NET_POLLER_H_

#include <map>
#include <vector>

#include "mytinyhttpd/net/event_loop.h"
#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

namespace net {

class Channel;

class Poller : public noncopyable {
 public:
  typedef std::vector<Channel*> ChannelList;

  Poller(EventLoop* loop);
  virtual ~Poller();

  virtual Timestamp Poll(int timeout_ms, ChannelList* active_channels) = 0;

  virtual void UpdateChannel(Channel* channel) = 0;

  virtual void RemoveChannel(Channel* channel) = 0;

  virtual bool HasChannel(Channel* channel) const;

  static Poller* NewDefaultPoller(EventLoop* loop);

  void AssertInLoopThread() const { loop_->AssertInLoopThread(); }

 protected:
  typedef std::map<int, Channel*> ChannelMap;
  ChannelMap channels_;

 private:
  EventLoop* loop_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_POLLER_H_
