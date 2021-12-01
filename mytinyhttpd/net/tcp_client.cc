#include "mytinyhttpd/net/tcp_client.h"

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/net/connector.h"
#include "mytinyhttpd/net/event_loop.h"
#include "mytinyhttpd/net/socket_ops.h"

namespace mytinyhttpd {

namespace net {

namespace detail {

void RemoveConnection(EventLoop* loop, const TcpConnectionPtr& conn) {
  loop->QueueInLoop(std::bind(&TcpConnection::ConnectDestroyed, conn));
}

void RemoveConnector(const ConnectorPtr& connector) {}

}  // namespace detail

TcpClient::TcpClient(EventLoop* loop, const InetAddress& serverAddr,
                     const std::string& nameArg)
    : loop_(CHECK_NOTNULL(loop)),
      connector_(new Connector(loop, serverAddr)),
      name_(nameArg),
      connection_callback_(DefaultConnectionCallback),
      message_callback_(DefaultMessageCallback),
      is_retry_(false),
      is_connect_(true),
      next_conn_id_(1) {
  connector_->SetNewConnectionCallback(
      std::bind(&TcpClient::NewConnection, this, _1));
  // FIXME SetConnectFailedCallback
  LOG_INFO << "TcpClient::TcpClient[" << name_ << "] - connector "
           << connector_.get();
}

TcpClient::~TcpClient() {
  LOG_INFO << "TcpClient::~TcpClient[" << name_ << "] - connector "
           << connector_.get();
  TcpConnectionPtr conn;
  bool unique = false;
  {
    MutexLockGuard lock(mutex_);
    unique = connection_.unique();
    conn = connection_;
  }
  if (conn) {
    assert(loop_ == conn->loop());
    CloseCallback cb = std::bind(&detail::RemoveConnection, loop_, _1);
    loop_->RunInLoop(std::bind(&TcpConnection::SetCloseCallback, conn, cb));
    if (unique) {
      conn->ForceClose();
    }
  } else {
    connector_->Stop();
    loop_->RunAfter(1, std::bind(&detail::RemoveConnector, connector_));
  }
}

void TcpClient::Connect() {
  LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to "
           << connector_->server_address().ToIpPort();
  is_connect_ = true;
  connector_->Start();
}

void TcpClient::Disconnect() {
  is_connect_ = false;

  {
    MutexLockGuard lock(mutex_);
    if (connection_) {
      connection_->Shutdown();
    }
  }
}

void TcpClient::Stop() {
  is_connect_ = false;
  connector_->Stop();
}

void TcpClient::NewConnection(int sockfd) {
  loop_->AssertInLoopThread();
  InetAddress peer_addr(socket::GetPeerAddr(sockfd));
  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", peer_addr.ToIpPort().c_str(),
           next_conn_id_);
  ++next_conn_id_;
  std::string conn_name = name_ + buf;

  InetAddress local_addr(socket::GetLocalAddr(sockfd));
  // FIXME poll with zero timeout to double confirm the new connection
  TcpConnectionPtr conn(
      new TcpConnection(loop_, conn_name, sockfd, local_addr, peer_addr));

  conn->SetConnectionCallback(connection_callback_);
  conn->SetMessageCallback(message_callback_);
  conn->SetWriteCompleteCallback(write_complete_callback_);
  conn->SetCloseCallback(std::bind(&TcpClient::RemoveConnection, this, _1));
  {
    MutexLockGuard lock(mutex_);
    connection_ = conn;
  }
  conn->ConnectEstablished();
}

void TcpClient::RemoveConnection(const TcpConnectionPtr& conn) {
  loop_->AssertInLoopThread();
  assert(loop_ == conn->loop());

  {
    MutexLockGuard lock(mutex_);
    assert(connection_ == conn);
    connection_.reset();
  }

  loop_->QueueInLoop(std::bind(&TcpConnection::ConnectDestroyed, conn));
  if (is_retry_ && is_connect_) {
    LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
             << connector_->server_address().ToIpPort();
    connector_->Restart();
  }
}

}  // namespace net

}  // namespace mytinyhttpd