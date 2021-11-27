#include "mytinyhttpd/net/event_loop_thread_pool.h"

#include <cstddef>

#include "mytinyhttpd/net/event_loop.h"
#include "mytinyhttpd/net/event_loop_thread.h"

namespace mytinyhttpd {

namespace net {

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop,
                                         const std::string& nameArg)
    : base_loop_(baseLoop),
      name_(nameArg),
      is_started_(false),
      num_threads_(0),
      next_(0) {}

void EventLoopThreadPool::Start(const ThreadInitCallback& init_callback) {
  assert(!is_started_);
  base_loop_->AssertInLoopThread();

  is_started_ = true;

  for (size_t i = 0; i < num_threads_; ++i) {
    char buf[name_.size() + 32];
    snprintf(buf, sizeof buf, "%s%lu", name_.c_str(), i);
    EventLoopThread* t = new EventLoopThread(init_callback, buf);
    // sometimes emplace_back is more effective than push_back
    threads_.emplace_back(std::unique_ptr<EventLoopThread>(t));
    loops_.emplace_back(t->StartLoop());
  }
  if (num_threads_ == 0 && init_callback) {
    init_callback(base_loop_);
  }
}

void EventLoopThreadPool::ResizeThreadNum(
    size_t num_threads, const ThreadInitCallback& init_callback) {
  assert(is_started_);
  base_loop_->AssertInLoopThread();

  size_t old_num_threads = threads_.size();
  if (num_threads < old_num_threads) {
    threads_.resize(num_threads);
    threads_.shrink_to_fit();
  } else if (num_threads > old_num_threads) {
    threads_.reserve(num_threads);
    for (size_t i = old_num_threads; i < num_threads_; ++i) {
      char buf[name_.size() + 32];
      snprintf(buf, sizeof buf, "%s%lu", name_.c_str(), i);
      EventLoopThread* t = new EventLoopThread(init_callback, buf);
      threads_.emplace_back(std::unique_ptr<EventLoopThread>(t));
      loops_.emplace_back(t->StartLoop());
    }
  }
  if (num_threads_ == 0 && init_callback) {
    init_callback(base_loop_);
  }
}

EventLoop* EventLoopThreadPool::GetNextLoop() {
  base_loop_->AssertInLoopThread();
  assert(is_started_);
  EventLoop* loop = base_loop_;

  if (!loops_.empty()) {
    // TODO: replace round-robin
    loop = loops_[next_];
    ++next_;
    if (static_cast<size_t>(next_) >= loops_.size()) {
      next_ = 0;
    }
  }
  return loop;
}

EventLoop* EventLoopThreadPool::GetLoopByHash(size_t hash_code) {
  base_loop_->AssertInLoopThread();
  EventLoop* loop = base_loop_;

  if (!loops_.empty()) {
    loop = loops_[hash_code % loops_.size()];
  }
  return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::GetAllLoops() {
  base_loop_->AssertInLoopThread();
  assert(is_started_);
  if (loops_.empty()) {
    return std::vector<EventLoop*>(1, base_loop_);
  } else {
    return loops_;
  }
}

}  // namespace net

}  // namespace mytinyhttpd