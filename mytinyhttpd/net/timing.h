#ifndef MYTINYHTTPD_NET_TIMING_H_
#define MYTINYHTTPD_NET_TIMING_H_

#include "mytinyhttpd/net/callbacks.h"
#include "mytinyhttpd/utils/noncopyable.h"

namespace mytinyhttpd {

namespace net {

class Timing : public noncopyable {
 public:
  Timing(int idle_seconds = 8) {}
  virtual ~Timing() {}

  virtual void OnTimer() = 0;

  virtual void OnConnection(const TcpConnectionPtr& conn) = 0;

  virtual void OnMessage(const TcpConnectionPtr& conn,
                         Timestamp receive_time) = 0;

  static Timing* NewDefaultTiming(int idle_seconds);
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_TIMING_H_