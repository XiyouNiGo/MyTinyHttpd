#include <mytinyhttpd/base/logging.h>
#include <mytinyhttpd/net/event_loop.h>
#include <mytinyhttpd/net/tcp_server.h>

using namespace mytinyhttpd;
using namespace mytinyhttpd::net;

class EchoServer {
 public:
  EchoServer(EventLoop* loop, const InetAddress& listen_addr)
      : server(loop, listen_addr, "EchoServer") {
    server.SetConnectionCallback(
        std::bind(&EchoServer::OnConnection, this, _1));
    server.SetMessageCallback(
        std::bind(&EchoServer::OnMessage, this, _1, _2, _3));
  }

  void Start() { server.Start(); }

 private:
  void OnConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << "EchoServer - " << conn->peer_address().ToIpPort() << " -> "
             << conn->local_address().ToIpPort() << " is "
             << (conn->IsConnected() ? "UP" : "DOWN");
  }

  void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
    std::string msg(buf->RetrieveAllAsString());
    LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
             << "data received at " << time.ToString();
    conn->Send(msg);
  }

  TcpServer server;
};

int main() {
  Logger::SetLogLevel(Logger::kDebug);
  LOG_INFO << "pid = " << getpid();
  EventLoop loop;
  InetAddress listen_addr(8888);
  EchoServer server(&loop, listen_addr);
  server.Start();
  loop.Loop();
}