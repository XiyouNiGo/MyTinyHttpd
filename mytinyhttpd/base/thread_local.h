#ifndef MYTINYHTTPD_BASE_THREAD_LOACL_H_

#include <pthread.h>

#include <boost/operators.hpp>

#include "mytinyhttpd/utils/constants.h"
#include "mytinyhttpd/utils/noncopyable.h"

namespace mytinyhttpd {

template <typename T>
class ThreadLocal : public noncopyable,
                    public boost::dereferenceable<ThreadLocal<T>, T*> {
 public:
  ThreadLocal() {
    CheckRetVal(pthread_key_create(&key_, &ThreadLocal::destructor), 0,
                "ThreadLocal Constructor");
  }

  ~ThreadLocal() {
    CheckRetVal(pthread_key_delete(key_), 0, "ThreadLocal Destructor");
  }

  T& operator*() const {
    T* value = static_cast<T*>(pthread_getspecific(key_));
    if (unlikely(!value)) {
      T* new_value = new T();
      CheckRetVal(pthread_setspecific(key_, new_value), 0,
                  "ThreadLocal operator*");
      value = new_value;
    }
    return *value;
  }

 private:
  static void destructor(void* x) {
    T* pval = static_cast<T*>(x);
    typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
    // tell the compiler not to optimize typedef
    (void)sizeof(T_must_be_complete_type);
    delete pval;
  }

 private:
  pthread_key_t key_;
};

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_THREAD_LOACL_H_