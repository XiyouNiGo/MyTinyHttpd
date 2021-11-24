#include "mytinyhttpd/net/epoll_poller.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/net/channel.h"

namespace mytinyhttpd {

namespace net {

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
static_assert(EPOLLIN == POLLIN, "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR, "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses same flag values as poll");

EPollPoller::EPollPoller(EventLoop* loop)
    : Poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
  if (epollfd_ < 0) {
    LOG_SYSFATAL << "EPollPoller::EPollPoller";
  }
}

EPollPoller::~EPollPoller() { ::close(epollfd_); }

Timestamp EPollPoller::Poll(int timeout_ms, ChannelList* active_channels) {
  LOG_TRACE << "fd total count " << channels_.size();
  int num_events = ::epoll_wait(epollfd_, &*events_.begin(),
                                static_cast<int>(events_.size()), timeout_ms);
  int old_errno = errno;
  Timestamp now(Timestamp::Now());
  if (num_events > 0) {
    LOG_TRACE << num_events << " events happened";
    FillActiveChannels(num_events, active_channels);
    if (static_cast<size_t>(num_events) == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  } else if (num_events == 0) {
    LOG_TRACE << "nothing happened";
  } else if (old_errno != EINTR) {
    errno = old_errno;
    LOG_SYSERR << "EPollPoller::Poll()";
  }
  return now;
}

void EPollPoller::FillActiveChannels(int num_events,
                                     ChannelList* active_channels) const {
  assert(static_cast<size_t>(num_events) <= events_.size());
  for (int i = 0; i < num_events; ++i) {
    Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
    channel->SetRevents(static_cast<int>(events_[i].events));
    active_channels->push_back(channel);
#ifndef NDEBUG
    int fd = channel->fd();
    auto ch = channels_.find(fd);
    assert(ch != channels_.end());
    assert(ch->second == channel);
#endif
  }
}

void EPollPoller::UpdateChannel(Channel* channel) {
  Poller::AssertInLoopThread();
  const Channel::Status status = channel->status();
  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events()
            << " status = " << static_cast<int>(status);
  if (status == Channel::Status::kNew || status == Channel::Status::kDeleted) {
    // a new one, add with EPOLL_CTL_ADD
    int fd = channel->fd();
    if (status == Channel::Status::kNew) {
      assert(channels_.find(fd) == channels_.end());
      channels_[fd] = channel;
    } else {
      // kDeleted
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }
    channel->SetStatus(Channel::Status::kAdded);
    Update(EPOLL_CTL_ADD, channel);
  } else if (status == Channel::Status::kAdded) {
    // update existing one with EPOLL_CTL_MOD/DEL
    int fd = channel->fd();
    (void)fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(status == Channel::Status::kAdded);
    if (channel->IsNoneEvent()) {
      Update(EPOLL_CTL_DEL, channel);
      channel->SetStatus(Channel::Status::kDeleted);
    } else {
      Update(EPOLL_CTL_MOD, channel);
    }
  } else {
    LOG_FATAL << "unknown Channel::Status type: " << status;
  }
}

void EPollPoller::RemoveChannel(Channel* channel) {
  Poller::AssertInLoopThread();
  int fd = channel->fd();
  LOG_TRACE << "fd = " << fd;
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->IsNoneEvent());
  Channel::Status status = channel->status();
  assert(status == Channel::Status::kAdded ||
         status == Channel::Status::kDeleted);
  size_t n = channels_.erase(fd);
  (void)n;
  assert(n == 1);

  if (status == Channel::Status::kAdded) {
    Update(EPOLL_CTL_DEL, channel);
  }
  channel->SetStatus(Channel::Status::kNew);
}

void EPollPoller::Update(int operation, Channel* channel) {
  struct epoll_event event;
  ::bzero(&event, sizeof event);
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();
  LOG_TRACE << "epoll_ctl op = " << OperationToString(operation)
            << " fd = " << fd << " event = { " << channel->EventsToString()
            << " }";
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      LOG_SYSERR << "epoll_ctl op =" << OperationToString(operation)
                 << " fd =" << fd;
    } else {
      LOG_SYSFATAL << "epoll_ctl op =" << OperationToString(operation)
                   << " fd =" << fd;
    }
  }
}

const char* EPollPoller::OperationToString(int op) {
  switch (op) {
    case EPOLL_CTL_ADD:
      return "ADD";
    case EPOLL_CTL_DEL:
      return "DEL";
    case EPOLL_CTL_MOD:
      return "MOD";
    default:
      LOG_SYSFATAL << "epoll_ctl unknown op =" << op;
      return "Unknown Operation";
  }
}

}  // namespace net

}  // namespace mytinyhttpd