#ifndef MYTINYHTTPD_NET_EVENT_LOOP_H_
#define MYTINYHTTPD_NET_EVENT_LOOP_H_

#include <atomic>
#include <functional>
#include <vector>

#include "mytinyhttpd/base/current_thread.h"
#include "mytinyhttpd/base/mutex.h"
#include "mytinyhttpd/net/callbacks.h"
#include "mytinyhttpd/net/channel.h"
#include "mytinyhttpd/net/timer_id.h"
#include "mytinyhttpd/utils/any.h"
#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

namespace net {

class Poller;
class TimerQueue;

class EventLoop : public noncopyable {
 public:
  typedef std::function<void()> Functor;

  EventLoop();
  ~EventLoop();

  void Loop();

  void Quit();

  Timestamp poll_return_time() const { return poll_return_time_; }

  int64_t iteration() const { return iteration_; }

  void RunInLoop(Functor cb);
  void QueueInLoop(Functor cb);

  size_t queue_size() const;

  TimerId RunAt(Timestamp time, TimerCallback cb);

  TimerId RunAfter(double delay, TimerCallback cb);

  TimerId RunEvery(double interval, TimerCallback cb);

  void Cancel(TimerId timer_id);

  void Wakeup();
  void UpdateChannel(Channel* channel);
  void RemoveChannel(Channel* channel);
  bool HasChannel(Channel* channel);

  void AssertInLoopThread() {
    if (!IsInLoopThread()) {
      AbortNotInLoopThread();
    }
  }
  bool IsInLoopThread() const { return thread_id_ == CurrentThread::tid(); }
  bool IsCallingPendingFunctors() const { return is_calling_pending_functors_; }
  bool IsEventHandling() const { return is_event_handling_; }

  void SetContext(const Any& context) { context_ = context; }

  const Any context() const { return context_; }

  Any* mutable_context() { return &context_; }

  static EventLoop* t_loop_in_this_thread();

 private:
  void AbortNotInLoopThread();
  void HandleRead();  // used for waked up
  void DoPendingFunctors();

  void PrintActiveChannels() const;

  typedef std::vector<Channel*> ChannelList;

  bool is_looping_;
  std::atomic<bool> is_quit_;
  bool is_event_handling_;
  bool is_calling_pending_functors_;
  int64_t iteration_;
  const pid_t thread_id_;
  Timestamp poll_return_time_;
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerQueue> timer_queue_;
  int wakeup_fd_;
  std::unique_ptr<Channel> wakeup_channel_;
  Any context_;

  ChannelList active_channels_;
  Channel* current_active_channel_;

  mutable MutexLock mutex_;
  std::vector<Functor> pending_functors_ GUARDED_BY(mutex_);
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_EVENT_LOOP_H_
