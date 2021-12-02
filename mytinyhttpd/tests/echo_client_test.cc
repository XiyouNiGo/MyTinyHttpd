#include <mytinyhttpd/base/logging.h>
#include <mytinyhttpd/net/event_loop.h>
#include <mytinyhttpd/net/tcp_client.h>

using namespace mytinyhttpd;
using namespace mytinyhttpd::net;

class EchoClient {
 public:
  EchoClient(EventLoop* loop, const InetAddress& listen_addr)
      : loop_(loop), client_(loop, listen_addr, "EchoClient") {
    client_.SetConnectionCallback(
        std::bind(&EchoClient::OnConnection, this, _1));
    client_.SetMessageCallback(
        std::bind(&EchoClient::OnMessage, this, _1, _2, _3));
  }

  void Connect() { client_.Connect(); }

  void Stop() { client_.Stop(); }

 private:
  void OnConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << "EchoClient - " << conn->peer_address().ToIpPort() << " -> "
             << conn->local_address().ToIpPort() << " is "
             << (conn->IsConnected() ? "UP" : "DOWN");
    // gdb: Cannot access memory at address (SIGSEGV)
    // loop_->RunEvery(2, [&]() { conn->Send("Hello EchoServer"); });
    loop_->RunEvery(2,
                    [&]() { client_.connection()->Send("Hello EchoServer"); });
    
  }

  void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
    std::string msg(buf->RetrieveAllAsString());
    LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
             << "data received at " << time.ToString();
  }

  EventLoop* loop_;
  TcpClient client_;
};

int main() {
  Logger::SetLogLevel(Logger::kDebug);
  LOG_INFO << "pid = " << getpid();
  EventLoop loop;
  InetAddress listen_addr(8888);
  EchoClient client(&loop, listen_addr);
  client.Connect();
  loop.Loop();
}