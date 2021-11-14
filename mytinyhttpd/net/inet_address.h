#ifndef MYTINYHTTPD_NET_INET_ADDRESS_H_
#define MYTINYHTTPD_NET_INET_ADDRESS_H_

#include <netinet/in.h>

#include "mytinyhttpd/net/socket_ops.h"
#include "mytinyhttpd/utils/copyable.h"
#include "mytinyhttpd/utils/slice.h"

namespace mytinyhttpd {

namespace net {

class InetAddress : public copyable {
 public:
  explicit InetAddress(uint16_t port = 0, bool is_loopback_only = false,
                       bool is_ipv6 = false);

  InetAddress(Slice ip, uint16_t port, bool is_ipv6 = false);

  explicit InetAddress(const struct sockaddr_in& addr) : addr_(addr) {}

  explicit InetAddress(const struct sockaddr_in6& addr) : addr6_(addr) {}

  sa_family_t family() const { return addr_.sin_family; }
  std::string ToIp() const;
  std::string ToIpPort() const;
  uint16_t port() const;

  const struct sockaddr* GetSockAddr() const {
    return socket::sockaddr_cast(&addr6_);
  }
  void SetSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }

  uint32_t GetIpv4NetEndian() const;
  uint16_t GetPortNetEndian() const { return addr_.sin_port; }

  static bool Resolve(Slice hostname, InetAddress* result);

  void SetScopeId(uint32_t scope_id);

 private:
  union {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_INET_ADDRESS_H_
