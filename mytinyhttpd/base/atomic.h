#ifndef MYTINYHTTPD_BASE_ATOMIC_H_
#define MYTINYHTTPD_BASE_ATOMIC_H_

#include "mytinyhttpd/utils/noncopyable.h"

namespace mytinyhttpd {

template <typename T>
class Atomic : public noncopyable {
 public:
  Atomic(int value = 0) : value_(value) {}

  T Load() { return __sync_val_compare_and_swap(&value_, 0, 0); }

  void Store(T x) { return LoadStore(x); }

  T FetchAdd(T x) { return __sync_fetch_and_add(&value_, x); }

  T FetchSub(T x) { return __sync_fetch_and_sub(&value_, x); }

  T FetchAnd(T x) { return __sync_fetch_and_and(&value_, x); }

  T FetchOr(T x) { return __sync_fetch_and_or(&value_, x); }

  T FetchXor(T x) { return __sync_fetch_and_xor(&value_, x); }

  T AddFetch(T x) { return FetchAdd(x) + x; }

  T SubFetch(T x) { return FetchSub(x) - x; }

  T operator++(int) { return AddFetch(1); }

  T operator--(int) { return SubFetch(1); }

  void Add(T x) { FetchAdd(x); }

  void Sub(T x) { FetchSub(x); }

  T LoadStore(T x) { return __sync_lock_test_and_set(&value_, x); }

 private:
  volatile T value_;
};

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_ATOMIC_H_
