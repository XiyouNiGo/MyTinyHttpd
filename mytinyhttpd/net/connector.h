#ifndef MYTINYHTTPD_NET_CONNECTOR_H_
#define MYTINYHTTPD_NET_CONNECTOR_H__

#include <functional>
#include <memory>

#include "mytinyhttpd/net/inet_address.h"
#include "mytinyhttpd/utils/noncopyable.h"

namespace mytinyhttpd {

namespace net {

class Channel;
class EventLoop;

class Connector : public noncopyable,
                  public std::enable_shared_from_this<Connector> {
 public:
  typedef std::function<void(int sockfd)> NewConnectionCallback;

  Connector(EventLoop* loop, const InetAddress& server_addr);
  ~Connector();

  void SetNewConnectionCallback(const NewConnectionCallback& cb) {
    new_connection_callback_ = cb;
  }

  void Start();
  void Restart();
  void Stop();

  const InetAddress& server_address() const { return server_addr_; }

 private:
  enum State : char {
    kDisconnected,
    kConnecting,
    kConnected,
    kNumConnectorState
  };
  static_assert(static_cast<int>(kNumConnectorState) <= (1 << 7),
                "Connector::State overflow up");
  static const int kMaxRetryDelayMs = 30 * 1000;
  static const int kInitRetryDelayMs = 500;

  void SetState(State state) { state_ = state; }
  void StartInLoop();
  void StopInLoop();
  void Connect();
  void Connecting(int sockfd);
  void HandleWrite();
  void HandleError();
  void Retry(int sockfd);
  int RemoveAndResetChannel();
  void ResetChannel();

  EventLoop* loop_;
  InetAddress server_addr_;
  bool is_connect_;
  State state_;
  std::unique_ptr<Channel> channel_;  // pay attention to std::unique_ptr
  NewConnectionCallback new_connection_callback_;
  int retry_delay_ms_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_CONNECTOR_H_
