#ifndef MYTINYHTTPD_BASE_BOUNDED_BLOCKING_QUEUE_H_
#define MYTINYHTTPD_BASE_BOUNDED_BLOCKING_QUEUE_H_

#include <assert.h>

#include <boost/circular_buffer.hpp>

#include "mytinyhttpd/base/condition.h"
#include "mytinyhttpd/base/mutex.h"
#include "mytinyhttpd/utils/noncopyable.h"
#include "mytinyhttpd/utils/thread_safety_annotation.h"

namespace mytinyhttpd {

template <typename T>
class BoundedBlockingQueue : public noncopyable {
 public:
  BoundedBlockingQueue(int max_size)
      : mutex_(), not_empty_(mutex_), not_full_(mutex_), queue_(max_size) {}

  void push(const T& x) {
    MutexLockGuard lock(mutex_);
    while (queue_.full()) {
      not_full_.Wait();
    }
    assert(!queue_.full());
    queue_.push_back(x);
    not_empty_.Notify();
  }

  void push(T&& x) {
    MutexLockGuard lock(mutex_);
    while (queue_.full()) {
      not_full_.Wait();
    }
    assert(!queue_.full());
    queue_.push_back(std::move(x));
    not_empty_.Notify();
  }

  T pop() {
    MutexLockGuard lock(mutex_);
    while (queue_.empty()) {
      not_empty_.Wait();
    }
    assert(!queue_.empty());
    T front(std::move(queue_.front()));
    queue_.pop_front();
    not_full_.Notify();
    return front;
  }

  bool empty() const {
    MutexLockGuard lock(mutex_);
    return queue_.empty();
  }

  bool full() const {
    MutexLockGuard lock(mutex_);
    return queue_.full();
  }

  size_t size() const {
    MutexLockGuard lock(mutex_);
    return queue_.size();
  }

  size_t capacity() const {
    MutexLockGuard lock(mutex_);
    return queue_.capacity();
  }

 private:
  mutable MutexLock mutex_;
  Condition not_empty_ GUARDED_BY(mutex_);
  Condition not_full_ GUARDED_BY(mutex_);
  boost::circular_buffer<T> queue_ GUARDED_BY(mutex_);
};

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_BOUNDED_BLOCKING_QUEUE_H_
