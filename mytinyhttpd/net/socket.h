#ifndef MYTINYHTTPD_NET_SOCKET_H_
#define MYTINYHTTPD_NET_SOCKET_H_

#include <netinet/tcp.h>

#include "mytinyhttpd/net/inet_address.h"
#include "mytinyhttpd/utils/noncopyable.h"

namespace mytinyhttpd {

namespace net {

class Socket : public noncopyable {
 public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {}

  Socket(Socket&& x) noexcept : sockfd_(x.sockfd_) {}

  ~Socket() { socket::Close(sockfd_); }

  int socketfd() const { return sockfd_; }

  bool GetTcpInfo(struct tcp_info*) const;
  bool GetTcpInfoString(char* buf, int len) const;

  void BindAddress(const InetAddress& localaddr) {
    socket::BindOrDie(sockfd_, localaddr.GetSockAddr());
  }
  void Listen() { socket::ListenOrDie(sockfd_); }

  /// On success, returns a non-negative integer that is
  /// a descriptor for the accepted socket, which has been
  /// set to non-blocking and close-on-exec. *peeraddr is assigned.
  /// On error, -1 is returned, and *peeraddr is untouched.
  int Accept(InetAddress* peeraddr);

  void ShutdownWrite() { socket::ShutdownWrite(sockfd_); }

  // TCP_NODELAY
  void SetTcpNoDelay(bool on);

  // O_REUSEADDR
  void SetReuseAddr(bool on);

  /// SO_REUSEPORT
  void SetReusePort(bool on);

  /// SO_KEEPALIVE
  void SetKeepAlive(bool on);

 private:
  const int sockfd_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_SOCKET_H_