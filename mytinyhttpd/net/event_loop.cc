#include "mytinyhttpd/net/event_loop.h"

#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <algorithm>

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/base/mutex.h"
#include "mytinyhttpd/net/channel.h"
#include "mytinyhttpd/net/poller.h"
#include "mytinyhttpd/net/socket_ops.h"

namespace mytinyhttpd {

namespace net {

namespace detail {

__thread EventLoop* t_loop_in_this_thread = 0;

const int kPollTimeMs = 10000;

int CreateEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG_SYSERR << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe {
 public:
  IgnoreSigPipe() {
    ::signal(SIGPIPE, SIG_IGN);
    LOG_TRACE << "Ignore SIGPIPE";
  }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe ignore;

}  // namespace detail

EventLoop* EventLoop::t_loop_in_this_thread() {
  return detail::t_loop_in_this_thread;
}

EventLoop::EventLoop()
    : is_looping_(false),
      is_quit_(false),
      is_event_handling_(false),
      is_calling_pending_functors_(false),
      iteration_(0),
      thread_id_(CurrentThread::tid()),
      poller_(Poller::NewDefaultPoller(this)),
      // timerQueue_(new TimerQueue(this)),
      wakeup_fd_(detail::CreateEventfd()),
      wakeup_channel_(new Channel(this, wakeup_fd_)),
      current_active_channel_(nullptr) {
  LOG_DEBUG << "EventLoop created " << this << " in thread " << thread_id_;
  if (unlikely(detail::t_loop_in_this_thread)) {
    LOG_FATAL << "Another EventLoop " << detail::t_loop_in_this_thread
              << " exists in this thread " << thread_id_;
  } else {
    detail::t_loop_in_this_thread = this;
  }
  wakeup_channel_->SetReadCallback(std::bind(&EventLoop::HandleRead, this));
  wakeup_channel_->EnableReading();
}

EventLoop::~EventLoop() {
  LOG_DEBUG << "EventLoop " << this << " of thread " << thread_id_
            << " destructs in thread " << CurrentThread::tid();
  wakeup_channel_->DisableAll();
  wakeup_channel_->Remove();
  ::close(wakeup_fd_);
  detail::t_loop_in_this_thread = nullptr;
}

void EventLoop::Loop() {
  assert(!is_looping_);
  AssertInLoopThread();
  is_looping_ = true;
  is_quit_ = false;
  LOG_TRACE << "EventLoop " << this << " start looping";

  while (!is_quit_) {
    active_channels_.clear();
    poll_return_time_ = poller_->Poll(detail::kPollTimeMs, &active_channels_);
    ++iteration_;
    if (Logger::log_level() <= Logger::LogLevel::kTrace) {
      PrintActiveChannels();
    }
    is_event_handling_ = true;
    for (Channel* channel : active_channels_) {
      current_active_channel_ = channel;
      current_active_channel_->HandleEvent(poll_return_time_);
    }
    current_active_channel_ = nullptr;
    is_event_handling_ = false;
    DoPendingFunctors();
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  is_looping_ = false;
}

void EventLoop::Quit() {
  is_quit_ = true;
  // There is a chance that loop() just executes while(!quit_) and exits,
  // then EventLoop destructs, then we are accessing an invalid object.
  // Can be fixed using mutex_ in both places.
  if (!IsInLoopThread()) {
    Wakeup();
  }
}

void EventLoop::RunInLoop(Functor cb) {
  if (IsInLoopThread()) {
    cb();
  } else {
    QueueInLoop(std::move(cb));
  }
}

void EventLoop::QueueInLoop(Functor cb) {
  {
    MutexLockGuard lock(mutex_);
    pending_functors_.push_back(std::move(cb));
  }

  if (!IsInLoopThread() || is_calling_pending_functors_) {
    Wakeup();
  }
}

size_t EventLoop::queue_size() const {
  MutexLockGuard lock(mutex_);
  return pending_functors_.size();
}

// TimerId EventLoop::runAt(Timestamp time, TimerCallback cb) {
//   return timerQueue_->addTimer(std::move(cb), time, 0.0);
// }

// TimerId EventLoop::runAfter(double delay, TimerCallback cb) {
//   Timestamp time(addTime(Timestamp::now(), delay));
//   return runAt(time, std::move(cb));
// }

// TimerId EventLoop::runEvery(double interval, TimerCallback cb) {
//   Timestamp time(addTime(Timestamp::now(), interval));
//   return timerQueue_->addTimer(std::move(cb), time, interval);
// }

// void EventLoop::cancel(TimerId timerId) { return
// timerQueue_->cancel(timerId); }

void EventLoop::UpdateChannel(Channel* channel) {
  assert(channel->loop() == this);
  AssertInLoopThread();
  poller_->UpdateChannel(channel);
}

void EventLoop::RemoveChannel(Channel* channel) {
  assert(channel->loop() == this);
  AssertInLoopThread();
  if (is_event_handling_) {
    assert(current_active_channel_ == channel ||
           std::find(active_channels_.begin(), active_channels_.end(),
                     channel) == active_channels_.end());
  }
  poller_->RemoveChannel(channel);
}

bool EventLoop::HasChannel(Channel* channel) {
  assert(channel->loop() == this);
  AssertInLoopThread();
  return poller_->HasChannel(channel);
}

void EventLoop::AbortNotInLoopThread() {
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << thread_id_
            << ", current thread id = " << CurrentThread::tid();
}

void EventLoop::Wakeup() {
  uint64_t one = 1;
  ssize_t n = socket::Write(wakeup_fd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::Wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::HandleRead() {
  uint64_t one = 1;
  ssize_t n = socket::Read(wakeup_fd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::HandleRead() reads " << n
              << " bytes instead of 8 ";
  }
}

void EventLoop::DoPendingFunctors() {
  std::vector<Functor> functors;
  is_calling_pending_functors_ = true;

  {
    MutexLockGuard lock(mutex_);
    functors.swap(pending_functors_);
  }

  for (const Functor& functor : functors) {
    functor();
  }
  is_calling_pending_functors_ = false;
}

void EventLoop::PrintActiveChannels() const {
  for (const Channel* channel : active_channels_) {
    LOG_TRACE << "{" << channel->ReventsToString() << "} ";
  }
}

}  // namespace net

}  // namespace mytinyhttpd