#ifndef MYTINYHTTPD_NET_TCPCONNECTION_H_
#define MYTINYHTTPD_NET_TCPCONNECTION_H_

#include <netinet/tcp.h>

#include <boost/any.hpp>
#include <memory>

#include "mytinyhttpd/net/buffer.h"
#include "mytinyhttpd/net/callbacks.h"
#include "mytinyhttpd/net/inet_address.h"
#include "mytinyhttpd/utils/any.h"
#include "mytinyhttpd/utils/noncopyable.h"
#include "mytinyhttpd/utils/slice.h"

namespace mytinyhttpd {

namespace net {

class Channel;
class EventLoop;
class Socket;

class TcpConnection : public noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                const InetAddress& local_addr, const InetAddress& peer_addr);
  ~TcpConnection();

  EventLoop* loop() const { return loop_; }
  const std::string& name() const { return name_; }
  const InetAddress& local_address() const { return local_addr_; }
  const InetAddress& peer_address() const { return peer_addr_; }
  bool IsConnected() const { return state_ == kConnected; }
  bool IsDisconnected() const { return state_ == kDisconnected; }

  bool GetTcpInfo(struct tcp_info*) const;
  std::string GetTcpInfoString() const;

  // void Send(std::string&& message);
  void Send(const void* message, int len);
  void Send(const Slice& message);
  // void Send(Buffer&& message);
  void Send(Buffer* message);  // swap

  void Shutdown();
  void ShutdownAndForceCloseAfter(double seconds);

  void ForceClose();
  void ForceCloseWithDelay(double seconds);
  void SetTcpNoDelay(bool on);

  void StartRead();
  void StopRead();
  bool IsReading() const { return is_reading_; };

  void SetContext(const Any& context) { context_ = context; }
  void SetTimingContext(const Any& timing_context) {
    timing_context_ = timing_context;
  }

  const Any& context() const { return context_; }
  const Any& timing_context() const { return timing_context_; }

  Any* mutable_context() { return &context_; }
  Any* mutable_timing_context() { return &timing_context_; }

  void SetConnectionCallback(const ConnectionCallback& cb) {
    connection_callback_ = cb;
  }

  void SetMessageCallback(const MessageCallback& cb) { message_callback_ = cb; }

  void SetWriteCompleteCallback(const WriteCompleteCallback& cb) {
    write_complete_callback_ = cb;
  }

  void SetHighWaterMarkCallback(const HighWaterMarkCallback& cb,
                                size_t high_water_mark) {
    high_water_mark_callback_ = cb;
    high_water_mark_ = high_water_mark;
  }

  void SetCloseCallback(const CloseCallback& cb) { close_callback_ = cb; }

  Buffer* input_buffer() { return &input_buffer_; }

  Buffer* output_buffer() { return &output_buffer_; }

  void ConnectEstablished();
  void ConnectDestroyed();

 private:
  enum State : char {
    kDisconnected,
    kConnecting,
    kConnected,
    kDisconnecting,
    kNumTcpConnectionState
  };
  static_assert(static_cast<int>(kNumTcpConnectionState) <= (1 << 7),
                "TcpConnection::State overflow up");

  void HandleRead(Timestamp receive_time);
  void HandleWrite();
  void HandleClose();
  void HandleError();

  void SendInLoop(Buffer&& message);
  void SendInLoop(std::string&& message);
  void SendInLoop(const Slice& message);
  void SendInLoop(const void* message, size_t len);

  void ShutdownInLoop();
  void ShutdownAndForceCloseInLoop(double seconds);
  void ForceCloseInLoop();

  void SetState(State s) { state_ = s; }
  const char* StateToString() const;

  void StartReadInLoop();
  void StopReadInLoop();

  EventLoop* loop_;
  const std::string name_;
  State state_;
  bool is_reading_;

  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  const InetAddress local_addr_;
  const InetAddress peer_addr_;

  ConnectionCallback connection_callback_;
  MessageCallback message_callback_;
  WriteCompleteCallback write_complete_callback_;
  HighWaterMarkCallback high_water_mark_callback_;
  CloseCallback close_callback_;

  size_t high_water_mark_;
  Buffer input_buffer_;
  Buffer output_buffer_;  // TODO: use list<Buffer> as output buffer.
  Any context_;
  Any timing_context_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_TCPCONNECTION_H_
