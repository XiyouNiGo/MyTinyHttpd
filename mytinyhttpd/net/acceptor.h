#ifndef MYTINYHTTPD_NET_ACCEPTOR_H_
#define MYTINYHTTPD_NET_ACCEPTOR_H_

#include <functional>

#include "mytinyhttpd/net/channel.h"
#include "mytinyhttpd/net/socket.h"
#include "mytinyhttpd/utils/noncopyable.h"

namespace mytinyhttpd {

namespace net {

class EventLoop;
class InetAddress;

class Acceptor : public noncopyable {
 public:
  typedef std::function<void(int sockfd, const InetAddress&)>
      NewConnectionCallback;

  Acceptor(EventLoop* loop, const InetAddress& listen_addr, bool is_reuseport);
  ~Acceptor();

  void SetNewConnectionCallback(const NewConnectionCallback& cb) {
    new_connection_callback_ = cb;
  }

  void Listen();

  bool IsListening() const { return is_listening_; }

 private:
  void HandleRead();

  EventLoop* loop_;
  Socket accept_socket_;
  Channel accept_channel_;
  NewConnectionCallback new_connection_callback_;
  bool is_listening_;
  // for limit the maximum number of concurrent connections
  int idle_fd_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_ACCEPTOR_H_
