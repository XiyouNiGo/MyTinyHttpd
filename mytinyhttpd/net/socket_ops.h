#ifndef MYTINYHTTPD_NET_SOCKET_OPS_H_
#define MYTINYHTTPD_NET_SOCKET_OPS_H_

#include <arpa/inet.h>

namespace mytinyhttpd {

namespace net {

namespace socket {

int CreateNonblockingOrDie(sa_family_t family);

int Connect(int sockfd, const struct sockaddr* addr);
void BindOrDie(int sockfd, const struct sockaddr* addr);
void ListenOrDie(int sockfd);
int Accept(int sockfd, struct sockaddr_in6* addr);
ssize_t Read(int sockfd, void* buf, size_t count);
ssize_t Readv(int sockfd, const struct iovec* iov, int iovcnt);
ssize_t Write(int sockfd, const void* buf, size_t count);
void Close(int sockfd);
void ShutdownWrite(int sockfd);

void ToIpPort(char* buf, size_t size, const struct sockaddr* addr);
void ToIp(char* buf, size_t size, const struct sockaddr* addr);

void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);

int GetSocketError(int sockfd);

inline const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr) {
  return static_cast<const struct sockaddr*>(static_cast<const void*>(addr));
}

inline struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr) {
  return static_cast<struct sockaddr*>(static_cast<void*>(addr));
}

inline const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr) {
  return static_cast<const struct sockaddr*>(static_cast<const void*>(addr));
}

inline const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr) {
  return static_cast<const struct sockaddr_in*>(static_cast<const void*>(addr));
}

inline const struct sockaddr_in6* sockaddr_in6_cast(
    const struct sockaddr* addr) {
  return static_cast<const struct sockaddr_in6*>(
      static_cast<const void*>(addr));
}

struct sockaddr_in6 GetLocalAddr(int sockfd);
struct sockaddr_in6 GetPeerAddr(int sockfd);
bool IsSelfConnect(int sockfd);

}  // namespace socket

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_SOCKET_OPS_H_
