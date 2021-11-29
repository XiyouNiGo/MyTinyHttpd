#ifndef MYTINYHTTPD_NET_TCPSERVER_H_
#define MYTINYHTTPD_NET_TCPSERVER_H_

#include <map>

#include "mytinyhttpd/base/atomic.h"
#include "mytinyhttpd/net/tcp_connnection.h"
#include "mytinyhttpd/utils/noncopyable.h"

namespace mytinyhttpd {

namespace net {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : public noncopyable {
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;
  enum Option : char { kNoReusePort, kReusePort, kNumTcpServerOption };
  static_assert(static_cast<int>(kNumTcpServerOption) <= (1 << 7),
                "TcpServer::Option overflow up");

  TcpServer(EventLoop* loop, const InetAddress& listen_addr,
            const std::string& name_arg, Option option = kNoReusePort);
  ~TcpServer();

  const std::string& ip_port() const { return ip_port_; }
  const std::string& name() const { return name_; }
  EventLoop* loop() const { return loop_; }

  void SetThreadNum(int numThreads);
  void SetThreadInitCallback(const ThreadInitCallback& cb) {
    thread_init_callback_ = cb;
  }

  std::shared_ptr<EventLoopThreadPool> thread_pool() { return thread_pool_; }

  void Start();

  void SetConnectionCallback(const ConnectionCallback& cb) {
    connection_callback_ = cb;
  }

  void SetMessageCallback(const MessageCallback& cb) { message_callback_ = cb; }

  void SetWriteCompleteCallback(const WriteCompleteCallback& cb) {
    write_complete_callback_ = cb;
  }

 private:
  void NewConnection(int sockfd, const InetAddress& peer_addr);
  void RemoveConnection(const TcpConnectionPtr& conn);
  void RemoveConnectionInLoop(const TcpConnectionPtr& conn);

  typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

  EventLoop* loop_;
  const std::string ip_port_;
  const std::string name_;
  std::unique_ptr<Acceptor> acceptor_;
  std::shared_ptr<EventLoopThreadPool> thread_pool_;
  ConnectionCallback connection_callback_;
  MessageCallback message_callback_;
  WriteCompleteCallback write_complete_callback_;
  ThreadInitCallback thread_init_callback_;
  Atomic<int32_t> started_;

  int next_conn_id_;
  ConnectionMap connections_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_TCPSERVER_H_
