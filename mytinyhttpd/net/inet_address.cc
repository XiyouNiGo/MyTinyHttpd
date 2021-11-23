#include "mytinyhttpd/net/inet_address.h"

#include <netdb.h>
#include <netinet/in.h>
#include <strings.h>

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/net/endian.h"
#include "mytinyhttpd/net/socket.h"

#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

namespace mytinyhttpd {

namespace net {

static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
              "InetAddress is same size as sockaddr_in6");
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");

InetAddress::InetAddress(uint16_t port, bool is_loopback_only, bool is_ipv6) {
  static_assert(offsetof(InetAddress, addr6_) == 0, "addr6_ offset 0");
  static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset 0");
  if (is_ipv6) {
    ::bzero(&addr6_, sizeof addr6_);
    addr6_.sin6_family = AF_INET6;
    in6_addr ip = is_loopback_only ? in6addr_loopback : in6addr_any;
    addr6_.sin6_addr = ip;
    addr6_.sin6_port = socket::HostToNetwork16(port);
  } else {
    ::bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    in_addr_t ip = is_loopback_only ? kInaddrLoopback : kInaddrAny;
    addr_.sin_addr.s_addr = socket::HostToNetwork32(ip);
    addr_.sin_port = socket::HostToNetwork16(port);
  }
}

InetAddress::InetAddress(Slice ip, uint16_t port, bool is_ipv6) {
  if (is_ipv6 || strchr(ip.data(), ':')) {
    ::bzero(&addr6_, sizeof addr6_);
    socket::FromIpPort(ip.data(), port, &addr6_);
  } else {
    ::bzero(&addr_, sizeof addr_);
    socket::FromIpPort(ip.data(), port, &addr_);
  }
}

std::string InetAddress::ToIpPort() const {
  char buf[64] = {0};
  socket::ToIpPort(buf, sizeof buf, GetSockAddr());
  return buf;
}

std::string InetAddress::ToIp() const {
  char buf[64] = {0};
  socket::ToIp(buf, sizeof buf, GetSockAddr());
  return buf;
}

uint32_t InetAddress::GetIpv4NetEndian() const {
  assert(family() == AF_INET);
  return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::port() const {
  return socket::NetworkToHost16(GetPortNetEndian());
}

static __thread char t_resolve_buffer[64 * 1024];

bool InetAddress::Resolve(Slice hostname, InetAddress* out) {
  assert(out != nullptr);
  struct hostent hent;
  struct hostent* he = nullptr;
  int herrno = 0;
  ::bzero(&hent, sizeof(hent));

  int ret = ::gethostbyname_r(hostname.data(), &hent, t_resolve_buffer,
                              sizeof t_resolve_buffer, &he, &herrno);
  if (ret == 0 && he != nullptr) {
    assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
    out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
    return true;
  } else {
    if (ret) {
      LOG_SYSERR << "InetAddress::Resolve";
    }
    return false;
  }
}

void InetAddress::SetScopeId(uint32_t scope_id) {
  if (family() == AF_INET6) {
    // scope_id is meant to identify the network interface
    addr6_.sin6_scope_id = scope_id;
  }
}

}  // namespace net

}  // namespace mytinyhttpd