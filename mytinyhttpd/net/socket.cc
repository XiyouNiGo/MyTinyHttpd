#include "mytinyhttpd/net/socket.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <strings.h>

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/net/inet_address.h"
#include "mytinyhttpd/net/socket_ops.h"

namespace mytinyhttpd {

namespace net {

bool Socket::GetTcpInfo(struct tcp_info* tcpi) const {
  socklen_t len = sizeof(*tcpi);
  ::bzero(tcpi, len);
  return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}

bool Socket::GetTcpInfoString(char* buf, int len) const {
  struct tcp_info tcpi;
  bool ok = GetTcpInfo(&tcpi);
  if (ok) {
    snprintf(
        buf, len,
        "unrecovered=%u "
        "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
        "lost=%u retrans=%u rtt=%u rttvar=%u "
        "sshthresh=%u cwnd=%u total_retrans=%u",
        tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
        tcpi.tcpi_rto,          // Retransmit timeout in usec
        tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
        tcpi.tcpi_snd_mss, tcpi.tcpi_rcv_mss,
        tcpi.tcpi_lost,     // Lost packets
        tcpi.tcpi_retrans,  // Retransmitted packets out
        tcpi.tcpi_rtt,      // Smoothed round trip time in usec
        tcpi.tcpi_rttvar,   // Medium deviation
        tcpi.tcpi_snd_ssthresh, tcpi.tcpi_snd_cwnd,
        tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
  }
  return ok;
}

int Socket::Accept(InetAddress* peeraddr) {
  struct sockaddr_in6 addr;
  ::bzero(&addr, sizeof addr);
  int connfd = socket::Accept(sockfd_, &addr);
  if (connfd >= 0) {
    peeraddr->SetSockAddrInet6(addr);
  }
  return connfd;
}

void Socket::SetTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval,
               static_cast<socklen_t>(sizeof optval));
}

// O_REUSEADDR
void Socket::SetReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval,
               static_cast<socklen_t>(sizeof optval));
}

/// SO_REUSEPORT
void Socket::SetReusePort(bool on) {
#ifdef SO_REUSEPORT
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval,
                         static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && on) {
    LOG_SYSERR << "SO_REUSEPORT failed.";
  }
#else
  if (on) {
    LOG_ERROR << "SO_REUSEPORT is not supported.";
  }
#endif
}

/// SO_KEEPALIVE
void Socket::SetKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval,
               static_cast<socklen_t>(sizeof optval));
}

}  // namespace net

}  // namespace mytinyhttpd