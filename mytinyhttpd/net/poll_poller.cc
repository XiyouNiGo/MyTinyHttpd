#include "mytinyhttpd/net/poll_poller.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/net/channel.h"

namespace mytinyhttpd {

namespace net {

PollPoller::PollPoller(EventLoop* loop) : Poller(loop) {}

PollPoller::~PollPoller() = default;

Timestamp PollPoller::Poll(int timeout_ms, ChannelList* active_channels) {
  int num_events = ::poll(&*pollfds_.begin(), pollfds_.size(), timeout_ms);
  int old_errno = errno;
  Timestamp now(Timestamp::Now());
  if (num_events > 0) {
    LOG_TRACE << num_events << " events happened";
    FillActiveChannels(num_events, active_channels);
  } else if (num_events == 0) {
    LOG_TRACE << " nothing happened";
  } else {
    if (old_errno != EINTR) {
      errno = old_errno;
      LOG_SYSERR << "PollPoller::poll()";
    }
  }
  return now;
}

void PollPoller::FillActiveChannels(int num_events,
                                    ChannelList* active_channels) const {
  for (auto pfd = pollfds_.begin(); pfd != pollfds_.end() && num_events > 0;
       ++pfd) {
    if (pfd->revents > 0) {
      --num_events;
      auto ch = channels_.find(pfd->fd);
      assert(ch != channels_.end());
      Channel* channel = ch->second;
      assert(channel->fd() == pfd->fd);
      channel->SetRevents(pfd->revents);
      active_channels->push_back(channel);
    }
  }
}

void PollPoller::UpdateChannel(Channel* channel) {
  Poller::AssertInLoopThread();
  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
  if (channel->status() == Channel::Status::kNew) {
    // a new one, add to pollfds_
    assert(channels_.find(channel->fd()) == channels_.end());
    struct pollfd pfd;
    pfd.fd = channel->fd();
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    pollfds_.push_back(pfd);
    Channel::Status status = static_cast<Channel::Status>(pollfds_.size() - 1);
    channel->SetStatus(status);
    channels_[pfd.fd] = channel;
  } else {
    // update existing one
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    Channel::Status status = channel->status();
    assert(0 <= status && status < static_cast<int>(pollfds_.size()));
    struct pollfd& pfd = pollfds_[status];
    assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
    pfd.fd = channel->fd();
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    if (channel->IsNoneEvent()) {
      // ignore this pollfd
      pfd.fd = -channel->fd() - 1;
    }
  }
}

void PollPoller::RemoveChannel(Channel* channel) {
  Poller::AssertInLoopThread();
  LOG_TRACE << "fd = " << channel->fd();
  assert(channels_.find(channel->fd()) != channels_.end());
  assert(channels_[channel->fd()] == channel);
  assert(channel->IsNoneEvent());
  Channel::Status status = channel->status();
  assert(0 <= status && status < static_cast<int>(pollfds_.size()));
  const struct pollfd& pfd = pollfds_[status];
  (void)pfd;
  assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
  size_t n = channels_.erase(channel->fd());
  assert(n == 1);
  (void)n;
  if (static_cast<size_t>(status) == pollfds_.size() - 1) {
    pollfds_.pop_back();
  } else {
    // swap to the end, then pop_back
    int channel_at_end = pollfds_.back().fd;
    iter_swap(pollfds_.begin() + status, pollfds_.end() - 1);
    if (channel_at_end < 0) {
      channel_at_end = -channel_at_end - 1;
    }
    channels_[channel_at_end]->SetStatus(status);
    pollfds_.pop_back();
  }
}

}  // namespace net

}  // namespace mytinyhttpd