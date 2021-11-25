#ifndef MYTINYHTTPD_NET_TIMER_QUEUE_H_
#define MYTINYHTTPD_NET_TIMER_QUEUE_H_

#include <set>
#include <vector>

#include "mytinyhttpd/base/mutex.h"
#include "mytinyhttpd/net/callbacks.h"
#include "mytinyhttpd/net/channel.h"
#include "mytinyhttpd/utils/noncopyable.h"
#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {
namespace net {

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : public noncopyable {
 public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue();

  TimerId AddTimer(TimerCallback cb, Timestamp when, double interval);

  void Cancel(TimerId timer_id);

 private:
  typedef std::pair<Timestamp, Timer*>
      Entry;  // use std::pair to avoid repetition of
              // Timers which has the same Timestamp
  typedef std::set<Entry> TimerList;
  typedef std::pair<Timer*, int64_t> ActiveTimer;
  typedef std::set<ActiveTimer> ActiveTimerSet;

  void AddTimerInLoop(Timer* timer);
  void CancelInLoop(TimerId timerId);
  void HandleRead();
  std::vector<Entry> GetExpired(Timestamp now);
  void Reset(const std::vector<Entry>& expired, Timestamp now);

  bool Insert(Timer* timer);

  EventLoop* loop_;
  const int timerfd_;
  Channel timerfd_channel_;
  TimerList timers_;

  // for Cancel()
  ActiveTimerSet active_timers_;  // FIXME: no use variable?
  bool is_calling_expired_timers_;
  ActiveTimerSet canceling_timers_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_TIMER_QUEUE_H_
