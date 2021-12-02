#include "mytinyhttpd/net/tcp_connnection.h"

#include <errno.h>

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/net/callbacks.h"
#include "mytinyhttpd/net/channel.h"
#include "mytinyhttpd/net/event_loop.h"
#include "mytinyhttpd/net/socket.h"
#include "mytinyhttpd/net/socket_ops.h"
#include "mytinyhttpd/utils/weak_callback.h"

namespace mytinyhttpd {

namespace net {

void DefaultConnectionCallback(const TcpConnectionPtr& conn) {
  LOG_TRACE << conn->local_address().ToIpPort() << " -> "
            << conn->peer_address().ToIpPort() << " is "
            << (conn->IsConnected() ? "UP" : "DOWN");
}

void DefaultMessageCallback(const TcpConnectionPtr&, Buffer* buf,
                            Timestamp receive_time) {
  buf->RetrieveAll();
}

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name_arg,
                             int sockfd, const InetAddress& local_addr,
                             const InetAddress& peer_addr)
    : loop_(CHECK_NOTNULL(loop)),
      name_(name_arg),
      state_(kConnecting),
      is_reading_(true),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      local_addr_(local_addr),
      peer_addr_(peer_addr),
      high_water_mark_(64 * 1024 * 1024) {
  channel_->SetReadCallback(std::bind(&TcpConnection::HandleRead, this, _1));
  channel_->SetWriteCallback(std::bind(&TcpConnection::HandleWrite, this));
  channel_->SetCloseCallback(std::bind(&TcpConnection::HandleClose, this));
  channel_->SetErrorCallback(std::bind(&TcpConnection::HandleError, this));
  LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << this
            << " fd=" << sockfd;
  socket_->SetKeepAlive(true);
}

TcpConnection::~TcpConnection() {
  LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at " << this
            << " fd=" << channel_->fd() << " state=" << StateToString();
  assert(state_ == kDisconnected);
}

bool TcpConnection::GetTcpInfo(struct tcp_info* tcpi) const {
  return socket_->GetTcpInfo(tcpi);
}

std::string TcpConnection::GetTcpInfoString() const {
  char buf[1024];
  buf[0] = '\0';
  socket_->GetTcpInfoString(buf, sizeof buf);
  return buf;
}

// void TcpConnection::Send(std::string&& message) { Send(Slice(message)); }

void TcpConnection::Send(const void* data, int len) {
  Send(Slice(static_cast<const char*>(data), len));
}

void TcpConnection::Send(const Slice& message) {
  if (state_ == kConnected) {
    void (TcpConnection::*fp)(const Slice& message) =
        &TcpConnection::SendInLoop;
    loop_->RunInLoop(std::bind(fp, shared_from_this(), message.ToString()));
  }
}

// void TcpConnection::Send(Buffer&& message) {
//   if (state_ == kConnected) {
//     void (TcpConnection::*fp)(Buffer && message) =
//     &TcpConnection::SendInLoop; loop_->RunInLoop(std::bind(fp,
//     shared_from_this(), message));
//   }
// }

void TcpConnection::Send(Buffer* message) {
  if (state_ == kConnected) {
    void (TcpConnection::*fp)(const Slice& message) =
        &TcpConnection::SendInLoop;
    // FIXME: we have to copy Buffer here
    loop_->RunInLoop(
        std::bind(fp, shared_from_this(), message->RetrieveAllAsString()));
  }
}

void TcpConnection::SendInLoop(Buffer&& message) {
  SendInLoop(message.Peek(), message.readable_bytes());
  message.HasWritten(message.readable_bytes());
}

void TcpConnection::SendInLoop(std::string&& message) {
  SendInLoop(message.data(), message.size());
}

void TcpConnection::SendInLoop(const Slice& message) {
  SendInLoop(message.data(), message.size());
}

void TcpConnection::SendInLoop(const void* data, size_t len) {
  loop_->AssertInLoopThread();
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool fault_error = false;
  if (state_ == kDisconnected) {
    LOG_WARN << "disconnected, give up writing";
    return;
  }

  if (!channel_->IsWriting() && output_buffer_.readable_bytes() == 0) {
    nwrote = socket::Write(channel_->fd(), data, len);
    if (nwrote >= 0) {
      remaining = len - nwrote;
      if (remaining == 0 && write_complete_callback_) {
        loop_->QueueInLoop(
            std::bind(write_complete_callback_, shared_from_this()));
      }
    } else {
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        LOG_SYSERR << "TcpConnection::SendInLoop";
        if (errno == EPIPE || errno == ECONNRESET) {
          fault_error = true;
        }
      }
    }
  }

  assert(remaining <= len);
  if (!fault_error && remaining > 0) {
    size_t old_len = output_buffer_.readable_bytes();
    if (old_len + remaining >= high_water_mark_ && old_len < high_water_mark_ &&
        high_water_mark_callback_) {
      loop_->QueueInLoop(std::bind(high_water_mark_callback_,
                                   shared_from_this(), old_len + remaining));
    }
    output_buffer_.Append(static_cast<const char*>(data) + nwrote, remaining);
    if (!channel_->IsWriting()) {
      channel_->EnableWriting();
    }
  }
}

void TcpConnection::Shutdown() {
  if (state_ == kConnected) {
    SetState(kDisconnecting);
    loop_->RunInLoop(
        std::bind(&TcpConnection::ShutdownInLoop, shared_from_this()));
  }
}

void TcpConnection::ShutdownInLoop() {
  loop_->AssertInLoopThread();
  if (!channel_->IsWriting()) {
    socket_->ShutdownWrite();
  }
}

void TcpConnection::ShutdownAndForceCloseAfter(double seconds) {
  if (state_ == kConnected) {
    SetState(kDisconnecting);
    loop_->RunInLoop(
        std::bind(&TcpConnection::ShutdownAndForceCloseInLoop, this, seconds));
  }
}

void TcpConnection::ShutdownAndForceCloseInLoop(double seconds) {
  loop_->AssertInLoopThread();
  if (!channel_->IsWriting()) {
    socket_->ShutdownWrite();
  }
  loop_->RunAfter(seconds, MakeWeakCallback(shared_from_this(),
                                            &TcpConnection::ForceCloseInLoop));
}

void TcpConnection::ForceClose() {
  if (state_ == kConnected || state_ == kDisconnecting) {
    SetState(kDisconnecting);
    loop_->QueueInLoop(
        std::bind(&TcpConnection::ForceCloseInLoop, shared_from_this()));
  }
}

void TcpConnection::ForceCloseWithDelay(double seconds) {
  if (state_ == kConnected || state_ == kDisconnecting) {
    SetState(kDisconnecting);
    loop_->RunAfter(seconds, MakeWeakCallback(shared_from_this(),
                                              &TcpConnection::ForceClose));
  }
}

void TcpConnection::ForceCloseInLoop() {
  loop_->AssertInLoopThread();
  if (state_ == kConnected || state_ == kDisconnecting) {
    HandleClose();
  }
}

const char* TcpConnection::StateToString() const {
  switch (state_) {
    case kDisconnected:
      return "kDisconnected";
    case kConnecting:
      return "kConnecting";
    case kConnected:
      return "kConnected";
    case kDisconnecting:
      return "kDisconnecting";
    default:
      return "unknown state";
  }
}

void TcpConnection::SetTcpNoDelay(bool on) { socket_->SetTcpNoDelay(on); }

void TcpConnection::StartRead() {
  loop_->RunInLoop(std::bind(&TcpConnection::StartReadInLoop, this));
}

void TcpConnection::StartReadInLoop() {
  loop_->AssertInLoopThread();
  if (!is_reading_ || !channel_->IsReading()) {
    channel_->EnableReading();
    is_reading_ = true;
  }
}

void TcpConnection::StopRead() {
  loop_->RunInLoop(std::bind(&TcpConnection::StopReadInLoop, this));
}

void TcpConnection::StopReadInLoop() {
  loop_->AssertInLoopThread();
  if (is_reading_ || channel_->IsReading()) {
    channel_->DisableReading();
    is_reading_ = false;
  }
}

void TcpConnection::ConnectEstablished() {
  loop_->AssertInLoopThread();
  assert(state_ == kConnecting);
  SetState(kConnected);
  channel_->Tie(shared_from_this());
  channel_->EnableReading();

  connection_callback_(shared_from_this());
}

void TcpConnection::ConnectDestroyed() {
  loop_->AssertInLoopThread();
  if (state_ == kConnected) {
    SetState(kDisconnected);
    channel_->DisableAll();

    connection_callback_(shared_from_this());
  }
  channel_->Remove();
}

void TcpConnection::HandleRead(Timestamp receive_time) {
  loop_->AssertInLoopThread();
  int old_errno = 0;
  ssize_t n = input_buffer_.ReadFromFd(channel_->fd(), &old_errno);
  if (n > 0) {
    message_callback_(shared_from_this(), &input_buffer_, receive_time);
  } else if (n == 0) {
    HandleClose();
  } else {
    errno = old_errno;
    LOG_SYSERR << "TcpConnection::HandleRead";
    HandleError();
  }
}

void TcpConnection::HandleWrite() {
  loop_->AssertInLoopThread();
  if (channel_->IsWriting()) {
    ssize_t n = socket::Write(channel_->fd(), output_buffer_.Peek(),
                              output_buffer_.readable_bytes());
    if (n > 0) {
      output_buffer_.Retrieve(n);
      if (output_buffer_.readable_bytes() == 0) {
        channel_->DisableWriting();
        if (write_complete_callback_) {
          loop_->QueueInLoop(
              std::bind(write_complete_callback_, shared_from_this()));
        }
        // we dont call Shutdown right now when there still has data in Buffer
        if (state_ == kDisconnecting) {
          ShutdownInLoop();
        }
      }
    } else {
      LOG_SYSERR << "TcpConnection::HandleWrite";
    }
  } else {
    LOG_TRACE << "Connection fd = " << channel_->fd()
              << " is down, no more writing";
  }
}

void TcpConnection::HandleClose() {
  loop_->AssertInLoopThread();
  LOG_TRACE << "fd = " << channel_->fd() << " state = " << StateToString();
  assert(state_ == kConnected || state_ == kDisconnecting);
  SetState(kDisconnected);
  channel_->DisableAll();

  TcpConnectionPtr guard_this(shared_from_this());
  connection_callback_(guard_this);
  close_callback_(guard_this);
}

void TcpConnection::HandleError() {
  int err = socket::GetSocketError(channel_->fd());
  LOG_ERROR << "TcpConnection::HandleError [" << name_
            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

}  // namespace net

}  // namespace mytinyhttpd