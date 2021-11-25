
#include "mytinyhttpd/net/timer_queue.h"

#include <sys/timerfd.h>
#include <unistd.h>

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/net/event_loop.h"
#include "mytinyhttpd/net/timer.h"
#include "mytinyhttpd/net/timer_id.h"

namespace mytinyhttpd {

namespace net {

namespace detail {

int CreateTimerfd() {
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0) {
    LOG_SYSFATAL << "Failed in timerfd_create";
  }
  return timerfd;
}

struct timespec HowMuchTimeFromNow(Timestamp when) {
  int64_t microseconds = when.micro_seconds_since_epoch() -
                         Timestamp::Now().micro_seconds_since_epoch();
  if (microseconds < 100) {
    microseconds = 100;
  }
  struct timespec ts;
  ts.tv_sec =
      static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(
      (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
  return ts;
}

void ReadTimerfd(int timerfd, Timestamp now) {
  uint64_t howmany;
  ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
  LOG_TRACE << "TimerQueue::HandleRead() " << howmany << " at "
            << now.ToString();
  if (n != sizeof howmany) {
    LOG_ERROR << "TimerQueue::HandleRead() reads " << n
              << " bytes instead of 8";
  }
}

void ResetTimerfd(int timerfd, Timestamp expiration) {
  // wake up loop by timerfd_settime()
  struct itimerspec new_value;
  ::bzero(&new_value, sizeof new_value);
  new_value.it_value = HowMuchTimeFromNow(expiration);
  int ret = ::timerfd_settime(timerfd, 0, &new_value, nullptr);
  if (ret) {
    LOG_SYSERR << "timerfd_settime()";
  }
}

}  // namespace detail

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
      timerfd_(detail::CreateTimerfd()),
      timerfd_channel_(loop, timerfd_),
      timers_(),
      is_calling_expired_timers_(false) {
  timerfd_channel_.SetReadCallback(std::bind(&TimerQueue::HandleRead, this));
  timerfd_channel_.EnableReading();
}

TimerQueue::~TimerQueue() {
  timerfd_channel_.DisableAll();
  timerfd_channel_.Remove();
  ::close(timerfd_);
  // do not remove channel, since we're in EventLoop::dtor();
  for (const Entry& timer : timers_) {
    delete timer.second;
  }
}

TimerId TimerQueue::AddTimer(TimerCallback cb, Timestamp when,
                             double interval) {
  Timer* timer = new Timer(std::move(cb), when, interval);
  loop_->RunInLoop(std::bind(&TimerQueue::AddTimerInLoop, this, timer));
  return TimerId(timer, timer->sequence());
}

void TimerQueue::Cancel(TimerId timer_id) {
  loop_->RunInLoop(std::bind(&TimerQueue::CancelInLoop, this, timer_id));
}

void TimerQueue::AddTimerInLoop(Timer* timer) {
  loop_->AssertInLoopThread();
  bool earliest_changed = Insert(timer);

  if (earliest_changed) {
    detail::ResetTimerfd(timerfd_, timer->expiration());
  }
}

void TimerQueue::CancelInLoop(TimerId timer_id) {
  loop_->AssertInLoopThread();
  assert(timers_.size() == active_timers_.size());
  ActiveTimer timer(timer_id.timer_, timer_id.sequence_);
  ActiveTimerSet::iterator it = active_timers_.find(timer);
  if (it != active_timers_.end()) {
    size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
    assert(n == 1);
    (void)n;
    delete it->first;
    active_timers_.erase(it);
  } else if (is_calling_expired_timers_) {
    // user call TimerQueue::Cancel in TimerQueue::Run
    // this means we needn't reset if Timer is a periodic task
    canceling_timers_.insert(timer);
  }
  assert(timers_.size() == active_timers_.size());
}

void TimerQueue::HandleRead() {
  loop_->AssertInLoopThread();
  Timestamp now(Timestamp::Now());
  detail::ReadTimerfd(timerfd_, now);

  std::vector<Entry> expired = GetExpired(now);

  is_calling_expired_timers_ = true;
  canceling_timers_.clear();
  for (const Entry& it : expired) {
    it.second->Run();
  }
  is_calling_expired_timers_ = false;

  Reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::GetExpired(Timestamp now) {
  assert(timers_.size() == active_timers_.size());
  std::vector<Entry> expired;
  Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
  TimerList::iterator end = timers_.lower_bound(sentry);
  assert(end == timers_.end() || now < end->first);
  std::copy(timers_.begin(), end, back_inserter(expired));
  timers_.erase(timers_.begin(), end);

  for (const Entry& it : expired) {
    ActiveTimer timer(it.second, it.second->sequence());
    size_t n = active_timers_.erase(timer);
    assert(n == 1);
    (void)n;
  }

  assert(timers_.size() == active_timers_.size());
  return expired;
}

void TimerQueue::Reset(const std::vector<Entry>& expired, Timestamp now) {
  Timestamp next_expire;

  for (const Entry& it : expired) {
    ActiveTimer timer(it.second, it.second->sequence());
    if (it.second->IsRepeat() &&
        canceling_timers_.find(timer) == canceling_timers_.end()) {
      it.second->Restart(now);
      Insert(it.second);
    } else {
      delete it.second;
    }
  }

  if (!timers_.empty()) {
    next_expire = timers_.begin()->second->expiration();
  }

  if (next_expire.IsValid()) {
    detail::ResetTimerfd(timerfd_, next_expire);
  }
}

bool TimerQueue::Insert(Timer* timer) {
  loop_->AssertInLoopThread();
  assert(timers_.size() == active_timers_.size());
  bool earliest_changed = false;
  Timestamp when = timer->expiration();
  TimerList::iterator it = timers_.begin();
  if (it == timers_.end() || when < it->first) {
    earliest_changed = true;
  }
  {
    std::pair<TimerList::iterator, bool> result =
        timers_.insert(Entry(when, timer));
    assert(result.second);
    (void)result;
  }
  {
    std::pair<ActiveTimerSet::iterator, bool> result =
        active_timers_.insert(ActiveTimer(timer, timer->sequence()));
    assert(result.second);
    (void)result;
  }

  assert(timers_.size() == active_timers_.size());
  return earliest_changed;
}

}  // namespace net

}  // namespace mytinyhttpd
