#ifndef MYTINYHTTPD_BASE_CONDITION_H_
#define MYTINYHTTPD_BASE_CONDITION_H_

#include <pthread.h>

#include "mytinyhttpd/base/mutex.h"
#include "mytinyhttpd/utils/constants.h"
#include "mytinyhttpd/utils/noncopyable.h"
#include "mytinyhttpd/utils/thread_safety_annotation.h"

namespace mytinyhttpd {

class Condition : public noncopyable {
 public:
  explicit Condition(MutexLock& mutex) : mutex_(mutex) {
    CheckRetVal(pthread_cond_init(&cond_, NULL), 0, "Condition::Constructor");
  }

  ~Condition() {
    CheckRetVal(pthread_cond_destroy(&cond_), 0, "Condition::Destructor");
  }

  void Wait() {
    MutexLock::UnassignGuard guard(mutex_);
    CheckRetVal(pthread_cond_wait(&cond_, mutex_.mutex()), 0,
                "Condition::Wait");
  }

  bool WaitForSeconds(double seconds);

  void Notify() {
    CheckRetVal(pthread_cond_signal(&cond_), 0, "Condition::Notify");
  }

  void NotifyAll() {
    CheckRetVal(pthread_cond_broadcast(&cond_), 0, "Condition NotifyAll");
  }

 private:
  MutexLock& mutex_;
  pthread_cond_t cond_;
};

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_CONDITION_H_
