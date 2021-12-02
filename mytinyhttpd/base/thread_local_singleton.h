#ifndef MYTINYHTTPD_BASE_THREAD_LOCAL_SINGLETON_H_
#define MYTINYHTTPD_BASE_THREAD_LOCAL_SINGLETON_H_

#include <assert.h>
#include <pthread.h>

#include "mytinyhttpd/utils/noncopyable.h"

namespace mytinyhttpd {

template <typename T>
class ThreadLocalSingleton : public noncopyable {
 public:
  ThreadLocalSingleton() = delete;
  ~ThreadLocalSingleton() = delete;

  static T& Instance() {
    // we neednt to use pthread_once_t to guarantee singleton cause t_value_ is
    // of __thread attribute
    if (!t_value_) {
      t_value_ = new T();
    }
    return *t_value_;
  }

  static T* Pointer() { return t_value_; }

 private:
  static void Destructor(void* obj) {
    assert(obj == t_value_);
    typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
    (void)sizeof(T_must_be_complete_type);
    delete t_value_;
    t_value_ = nullptr;
  }

  static __thread T* t_value_;
};

template <typename T>
__thread T* ThreadLocalSingleton<T>::t_value_ = 0;

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_THREAD_LOCAL_SINGLETON_H_
