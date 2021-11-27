#include "mytinyhttpd/net/acceptor.h"

#include <gtest/gtest.h>

#include "mytinyhttpd/net/event_loop.h"
#include "mytinyhttpd/net/inet_address.h"

namespace mytinyhttpd {

namespace net {

EventLoop* g_loop;

void NewConnectionCallback(int sockfd, const InetAddress& peeraddr) {
  socket::Close(sockfd);
  g_loop->Quit();
}

TEST(AcceptTest, AllTest) {
  EventLoop loop;
  g_loop = &loop;
  InetAddress listen_addr(8888);
  Acceptor acceptor(&loop, listen_addr, false);
  acceptor.SetNewConnectionCallback(NewConnectionCallback);
  acceptor.Listen();
  loop.Loop();
}

}  // namespace net

}  // namespace mytinyhttpd