#ifndef MYTINYHTTPD_NET_TIMER_H_
#define MYTINYHTTPD_NET_TIMER_H_

#include <cstdint>

#include "mytinyhttpd/base/atomic.h"
#include "mytinyhttpd/net/callbacks.h"
#include "mytinyhttpd/utils/noncopyable.h"
#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

namespace net {

class Timer : public noncopyable {
 public:
  Timer(TimerCallback cb, Timestamp when, double interval)
      : callback_(std::move(cb)),
        expiration_(when),
        interval_(interval),
        is_repeat_(interval > 0.0),
        sequence_((++num_created_).Load()) {}

  void Run() const { callback_(); }

  Timestamp expiration() const { return expiration_; }
  bool IsRepeat() const { return is_repeat_; }
  int64_t sequence() const { return sequence_; }

  void Restart(Timestamp now);

  static int64_t num_created() { return num_created_.Load(); }

 private:
  const TimerCallback callback_;
  Timestamp expiration_;
  const double interval_;
  const bool is_repeat_;
  const int64_t sequence_;

  static Atomic<int64_t> num_created_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_TIMER_H_
