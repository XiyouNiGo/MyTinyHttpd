#include "mytinyhttpd/net/event_loop_thread.h"

#include "mytinyhttpd/net/event_loop.h"

namespace mytinyhttpd {

namespace net {

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const std::string& name)
    : loop_(nullptr),
      is_exiting_(false),
      thread_(std::bind(&EventLoopThread::ThreadFunc, this), name),
      mutex_(),
      cond_(mutex_),
      callback_(cb) {}

EventLoopThread::~EventLoopThread() {
  is_exiting_ = true;
  if (loop_ != nullptr) {
    loop_->Quit();
    thread_.Join();
  }
}

EventLoop* EventLoopThread::StartLoop() {
  assert(!thread_.IsStarted());
  thread_.Start();

  EventLoop* loop = nullptr;
  {
    MutexLockGuard lock(mutex_);
    while (loop_ == nullptr) {
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
    loop_ = nullptr;
  }
}

}  // namespace net

}  // namespace mytinyhttpd