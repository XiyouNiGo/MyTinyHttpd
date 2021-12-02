#ifndef MYTINYHTTPD_BASE_SINGLETON_H_
#define MYTINYHTTPD_BASE_SINGLETON_H_

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#include "mytinyhttpd/utils/noncopyable.h"

namespace mytinyhttpd {

namespace detail {

template <typename T>
struct HasNoDestroy {
  // if C has not function named no_destroy
  template <typename C>
  static char Helper(decltype(&C::no_destroy));
  // else
  template <typename C>
  static int32_t Helper(...);

  const static bool value = sizeof(Helper<T>(0)) == 1;
};

}  // namespace detail

// we suggest use magic static to replace this class
template <typename T>
class Singleton : public noncopyable {
 public:
  Singleton() = delete;
  ~Singleton() = delete;

  static T& Instance() {
    pthread_once(&ponce_, &Singleton::Init);
    assert(value_ != nullptr);
    return *value_;
  }

 private:
  static void Init() {
    value_ = new T();
    if (!detail::HasNoDestroy<T>::value) {
      ::atexit(Destroy);
    }
  }

  static void Destroy() {
    typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
    T_must_be_complete_type dummy;
    (void)dummy;

    delete value_;
    value_ = nullptr;
  }

 private:
  static pthread_once_t ponce_;
  static T* value_;
};

template <typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

template <typename T>
T* Singleton<T>::value_ = nullptr;

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_SINGLETON_H_
