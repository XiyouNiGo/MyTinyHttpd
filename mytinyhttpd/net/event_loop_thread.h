#ifndef MYTINYHTTPD_NET_EVENT_LOOP_THREAD_H_
#define MYTINYHTTPD_NET_EVENT_LOOP_THREAD_H_

#include "mytinyhttpd/base/condition.h"
#include "mytinyhttpd/base/mutex.h"
#include "mytinyhttpd/base/thread.h"

namespace mytinyhttpd {

namespace net {

class EventLoop;

class EventLoopThread : public noncopyable {
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;

  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                  const std::string& name = std::string());
  ~EventLoopThread();
  EventLoop* StartLoop();

 private:
  void ThreadFunc();

  EventLoop* loop_ GUARDED_BY(mutex_);
  bool is_exiting_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_ GUARDED_BY(mutex_);
  ThreadInitCallback callback_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_EVENT_LOOP_THREAD_H_
