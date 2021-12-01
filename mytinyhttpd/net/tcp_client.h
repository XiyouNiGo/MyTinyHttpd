#ifndef MYTINYHTTPD_NET_TCPCLIENT_H_
#define MYTINYHTTPD_NET_TCPCLIENT_H_

#include "mytinyhttpd/base/mutex.h"
#include "mytinyhttpd/net/tcp_connnection.h"

namespace mytinyhttpd {

namespace net {

class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;

class TcpClient : public noncopyable {
 public:
  TcpClient(EventLoop* loop, const InetAddress& server_addr,
            const std::string& name_arg);
  ~TcpClient();

  void Connect();
  void Disconnect();
  void Stop();

  TcpConnectionPtr connection() const {
    MutexLockGuard lock(mutex_);
    return connection_;
  }

  EventLoop* loop() const { return loop_; }
  bool IsRetry() const { return is_retry_; }
  void EnableRetry() { is_retry_ = true; }

  const std::string& name() const { return name_; }

  void SetConnectionCallback(ConnectionCallback cb) {
    connection_callback_ = std::move(cb);
  }

  void SetMessageCallback(MessageCallback cb) {
    message_callback_ = std::move(cb);
  }

  void SetWriteCompleteCallback(WriteCompleteCallback cb) {
    write_complete_callback_ = std::move(cb);
  }

 private:
  void NewConnection(int sockfd);
  void RemoveConnection(const TcpConnectionPtr& conn);

  EventLoop* loop_;
  ConnectorPtr connector_;
  const std::string name_;
  ConnectionCallback connection_callback_;
  MessageCallback message_callback_;
  WriteCompleteCallback write_complete_callback_;
  bool is_retry_;
  bool is_connect_;
  int next_conn_id_;
  mutable MutexLock mutex_;
  TcpConnectionPtr connection_ GUARDED_BY(mutex_);
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_TCPCLIENT_H_
