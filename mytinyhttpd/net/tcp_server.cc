#include "mytinyhttpd/net/tcp_server.h"

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/net/acceptor.h"
#include "mytinyhttpd/net/event_loop.h"
#include "mytinyhttpd/net/event_loop_thread_pool.h"
#include "mytinyhttpd/net/socket_ops.h"

namespace mytinyhttpd {

namespace net {

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listen_addr,
                     const std::string& name_arg, Option option)
    : loop_(CHECK_NOTNULL(loop)),
      ip_port_(listen_addr.ToIpPort()),
      name_(name_arg),
      acceptor_(new Acceptor(loop, listen_addr, option == kReusePort)),
      thread_pool_(new EventLoopThreadPool(loop, name_)),
      connection_callback_(DefaultConnectionCallback),
      message_callback_(DefaultMessageCallback),
      next_conn_id_(1) {
  acceptor_->SetNewConnectionCallback(
      std::bind(&TcpServer::NewConnection, this, _1, _2));
}

TcpServer::~TcpServer() {
  loop_->AssertInLoopThread();
  LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

  for (auto& item : connections_) {
    TcpConnectionPtr conn(item.second);
    item.second.reset();
    conn->loop()->RunInLoop(std::bind(&TcpConnection::ConnectDestroyed, conn));
  }
}

void TcpServer::SetThreadNum(int numThreads) {
  assert(0 <= numThreads);
  thread_pool_->SetThreadNum(numThreads);
}

void TcpServer::Start() {
  if (started_.FetchSet(1) == 0) {
    thread_pool_->Start(thread_init_callback_);

    assert(!acceptor_->IsListening());
    loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
  }
}

void TcpServer::NewConnection(int sockfd, const InetAddress& peer_addr) {
  loop_->AssertInLoopThread();
  EventLoop* io_loop = thread_pool_->GetNextLoop();
  char buf[64];
  snprintf(buf, sizeof buf, "-%s#%d", ip_port_.c_str(), next_conn_id_);
  ++next_conn_id_;
  std::string conn_name = name_ + buf;

  LOG_INFO << "TcpServer::NewConnection [" << name_ << "] - new connection ["
           << conn_name << "] from " << peer_addr.ToIpPort();
  InetAddress local_addr(socket::GetLocalAddr(sockfd));
  // FIXME poll with zero timeout to double confirm the new connection
  TcpConnectionPtr conn(
      new TcpConnection(io_loop, conn_name, sockfd, local_addr, peer_addr));
  connections_[conn_name] = conn;
  conn->SetConnectionCallback(connection_callback_);
  conn->SetMessageCallback(message_callback_);
  conn->SetWriteCompleteCallback(write_complete_callback_);
  conn->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, this, _1));
  io_loop->RunInLoop(std::bind(&TcpConnection::ConnectEstablished, conn));
}

void TcpServer::RemoveConnection(const TcpConnectionPtr& conn) {
  loop_->RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr& conn) {
  loop_->AssertInLoopThread();
  LOG_INFO << "TcpServer::RemoveConnectionInLoop [" << name_
           << "] - connection " << conn->name();
  size_t n = connections_.erase(conn->name());
  (void)n;
  assert(n == 1);
  EventLoop* io_loop = conn->loop();
  io_loop->QueueInLoop(std::bind(&TcpConnection::ConnectDestroyed, conn));
}

}  // namespace net

}  // namespace mytinyhttpd