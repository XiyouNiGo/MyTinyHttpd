#ifndef MYTINYHTTPD_BASE_COUNT_DOWN_LATCH_H_
#define MYTINYHTTPD_BASE_COUNT_DOWN_LATCH_H_

#include "mytinyhttpd/base/condition.h"
#include "mytinyhttpd/base/mutex.h"
#include "mytinyhttpd/utils/noncopyable.h"
#include "mytinyhttpd/utils/thread_safety_annotation.h"

namespace mytinyhttpd {

class CountDownLatch : public noncopyable {
 public:
  explicit CountDownLatch(int count)
      : mutex_(), condition_(mutex_), count_(count) {}

  void Wait() {
    MutexLockGuard lock(mutex_);
    while (count_ > 0) {
      condition_.Wait();
    }
  }

  void CountDown() {
    MutexLockGuard lock(mutex_);
    --count_;
    if (count_ == 0) {
      condition_.NotifyAll();
    }
  }

  int count() const {
    MutexLockGuard lock(mutex_);
    return count_;
  }

 private:
  mutable MutexLock mutex_;
  Condition condition_ GUARDED_BY(mutex_);
  int count_ GUARDED_BY(mutex_);
};

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_COUNT_DOWN_LATCH_H_