#ifndef MYTINYHTTPD_BASE_MUTEX_H_
#define MYTINYHTTPD_BASE_MUTEX_H_

#include <assert.h>
#include <pthread.h>
#include <sys/types.h>

#include "mytinyhttpd/base/current_thread.h"
#include "mytinyhttpd/utils/constants.h"
#include "mytinyhttpd/utils/noncopyable.h"
#include "mytinyhttpd/utils/thread_safety_annotation.h"

namespace mytinyhttpd {

class CAPABILITY("mutex") MutexLock : noncopyable {
 public:
  MutexLock() : holder_(0) {
    CheckRetVal(pthread_mutex_init(&mutex_, NULL), 0, "MutexLock Constructor");
  }

  ~MutexLock() {
    assert(holder_ == 0);
    CheckRetVal(pthread_mutex_destroy(&mutex_), 0, "MutexLock Destructor");
  }

  bool IsLockedByThisThread() const { return holder_ == CurrentThread::Tid(); }

  void AssertLocked() const ASSERT_CAPABILITY(this) {
    assert(IsLockedByThisThread());
  }

 private:
  void Lock() ACQUIRE() {
    CheckRetVal(pthread_mutex_lock(&mutex_), 0, "MutexLock Lock");
    AssignHolder();
  }

  void Unlock() RELEASE() {
    UnassignHolder();
    CheckRetVal(pthread_mutex_unlock(&mutex_), 0, "MutexLock Unlock");
  }

  pthread_mutex_t* mutex() { return &mutex_; }

  void UnassignHolder() { holder_ = 0; }

  void AssignHolder() { holder_ = CurrentThread::Tid(); }
  class UnassignGuard : noncopyable {
   public:
    explicit UnassignGuard(MutexLock& lock) : lock_(lock) {
      lock_.UnassignHolder();
    }

    ~UnassignGuard() { lock_.AssignHolder(); }

   private:
    MutexLock& lock_;
  };

#define UnassignGuard(x) static_assert(0, "Missing guard object name")

  pthread_mutex_t mutex_;
  pid_t holder_;

  friend class Condition;
  friend class MutexLockGuard;
};

class SCOPED_CAPABILITY MutexLockGuard : noncopyable {
 public:
  explicit MutexLockGuard(MutexLock& mutex) ACQUIRE(mutex) : mutex_(mutex) {
    mutex_.Lock();
  }

  ~MutexLockGuard() RELEASE() { mutex_.Unlock(); }

 private:
  MutexLock& mutex_;
};

#define MutexLockGuard(x) static_assert(0, "Missing guard object name")

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_MUTEX_H_