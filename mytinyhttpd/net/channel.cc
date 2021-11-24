#include "mytinyhttpd/net/channel.h"

#include <poll.h>

#include <sstream>

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/net/event_loop.h"

namespace mytinyhttpd {

namespace net {

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      status_(kNew),
      is_log_hup_(true),
      is_tied_(false),
      is_event_handling_(false),
      is_added_to_loop_(false) {}

Channel::~Channel() {
  assert(!is_event_handling_);
  assert(!is_added_to_loop_);
  if (loop_->IsInLoopThread()) {
    assert(!loop_->HasChannel(this));
  }
}

void Channel::Tie(const std::shared_ptr<void>& obj) {
  tie_ = obj;
  is_tied_ = true;
}

void Channel::Update() {
  is_added_to_loop_ = true;
  loop_->UpdateChannel(this);
}

void Channel::Remove() {
  assert(IsNoneEvent());
  is_added_to_loop_ = false;
  loop_->RemoveChannel(this);
}

void Channel::HandleEvent(Timestamp receiveTime) {
  std::shared_ptr<void> guard;
  if (is_tied_) {
    guard = tie_.lock();
    if (guard) {
      HandleEventWithGuard(receiveTime);
    }
  } else {
    HandleEventWithGuard(receiveTime);
  }
}

void Channel::HandleEventWithGuard(Timestamp receiveTime) {
  is_event_handling_ = true;
  LOG_TRACE << ReventsToString();
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    if (is_log_hup_) {
      LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
    }
    if (close_callback_) close_callback_();
  }

  if (revents_ & POLLNVAL) {
    LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
  }

  if (revents_ & (POLLERR | POLLNVAL)) {
    if (error_callback_) error_callback_();
  }
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (read_callback_) read_callback_(receiveTime);
  }
  if (revents_ & POLLOUT) {
    if (write_callback_) write_callback_();
  }
  is_event_handling_ = false;
}

std::string Channel::ReventsToString() const {
  return EventsToString(fd_, revents_);
}

std::string Channel::EventsToString() const {
  return EventsToString(fd_, events_);
}

std::string Channel::EventsToString(int fd, int ev) {
  std::ostringstream oss;
  oss << fd << ": ";
  if (ev & POLLIN) oss << "IN ";
  if (ev & POLLPRI) oss << "PRI ";
  if (ev & POLLOUT) oss << "OUT ";
  if (ev & POLLHUP) oss << "HUP ";
  if (ev & POLLRDHUP) oss << "RDHUP ";
  if (ev & POLLERR) oss << "ERR ";
  if (ev & POLLNVAL) oss << "NVAL ";

  return oss.str();
}

}  // namespace net

}  // namespace mytinyhttpd