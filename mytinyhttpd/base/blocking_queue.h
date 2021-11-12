#ifndef MYTINYHTTPD_BASE_BLOCKING_QUEUE_H_
#define MYTINYHTTPD_BASE_BLOCKING_QUEUE_H_

#include <assert.h>

#include <queue>

#include "mytinyhttpd/base/condition.h"
#include "mytinyhttpd/base/mutex.h"
#include "mytinyhttpd/utils/noncopyable.h"
#include "mytinyhttpd/utils/thread_safety_annotation.h"

namespace mytinyhttpd {

template <typename T>
class BlockingQueue : public noncopyable {
 public:
  BlockingQueue() : mutex_(), not_empty_(mutex_), queue_() {}

  void push(const T& x) {
    MutexLockGuard lock(mutex_);
    queue_.push(x);
    not_empty_.Notify();
  }

  void push(T&& x) {
    MutexLockGuard lock(mutex_);
    queue_.push(std::move(x));
    not_empty_.Notify();
  }

  T pop() {
    MutexLockGuard lock(mutex_);
    while (queue_.empty()) {
      not_empty_.Wait();
    }
    assert(!queue_.empty());
    T front(std::move(queue_.front()));
    queue_.pop();
    return front;
  }

  std::queue<T> clear() {
    std::queue<T> q;
    {
      MutexLockGuard lock(mutex_);
      q = std::move(queue_);
      assert(queue_.empty());
    }
    return q;
  }

  size_t size() const {
    MutexLockGuard lock(mutex_);
    return queue_.size();
  }

 private:
  mutable MutexLock mutex_;
  Condition not_empty_ GUARDED_BY(mutex_);
  std::queue<T> queue_ GUARDED_BY(mutex_);
};

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_BLOCKING_QUEUE_H_
