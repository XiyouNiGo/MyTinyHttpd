#include "mytinyhttpd/net/connector.h"

#include <gtest/gtest.h>
#include <cstdio>
#include <memory>

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/net/event_loop.h"
#include "mytinyhttpd/net/inet_address.h"

namespace mytinyhttpd {

namespace net {

EventLoop* g_loop;

void NewConnectionCallback(int sockfd) {
  socket::Close(sockfd);
  g_loop->Quit();
}

TEST(ConnectorTest, AllTest) {
  Logger::SetLogLevel(Logger::kFatal);
  EventLoop loop;
  g_loop = &loop;
  InetAddress server_addr("127.0.0.1", 8888);
  std::shared_ptr<Connector> connector(new Connector(&loop, server_addr));
  connector->SetNewConnectionCallback(NewConnectionCallback);
  connector->Start();
  loop.Loop();
}

}  // namespace net

}  // namespace mytinyhttpd