#include "mytinyhttpd/net/event_loop_thread.h"

#include "mytinyhttpd/net/event_loop.h"

namespace mytinyhttpd {

namespace net {

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const std::string& name)
    : loop_(NULL),
      is_exiting_(false),
      thread_(std::bind(&EventLoopThread::ThreadFunc, this), name),
      mutex_(),
      cond_(mutex_),
      callback_(cb) {}

EventLoopThread::~EventLoopThread() {
  is_exiting_ = true;
  if (loop_ != NULL) {
    loop_->Quit();
    thread_.Join();
  }
}

EventLoop* EventLoopThread::StartLoop() {
  assert(!thread_.IsStarted());
  thread_.Start();

  EventLoop* loop = NULL;
  {
    MutexLockGuard lock(mutex_);
    while (loop_ == NULL) {
      cond_.Wait();
    }
    loop = loop_;
  }

  return loop;
}

void EventLoopThread::ThreadFunc() {
  EventLoop loop;

  if (callback_) {
    callback_(&loop);
  }

  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop;
    cond_.Notify();
  }

  loop.Loop();

  {
    MutexLockGuard lock(mutex_);
    loop_ = NULL;
  }
}

}  // namespace net

}  // namespace mytinyhttpd