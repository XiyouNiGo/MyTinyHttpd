#include "mytinyhttpd/net/connector.h"

#include <errno.h>

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/net/channel.h"
#include "mytinyhttpd/net/event_loop.h"
#include "mytinyhttpd/net/socket_ops.h"

namespace mytinyhttpd {

namespace net {

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& server_addr)
    : loop_(loop),
      server_addr_(server_addr),
      is_connect_(false),
      state_(kDisconnected),
      retry_delay_ms_(kInitRetryDelayMs) {
  LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector() {
  LOG_DEBUG << "dtor[" << this << "]";
  assert(!channel_);
}

void Connector::Start() {
  is_connect_ = true;
  loop_->RunInLoop(std::bind(&Connector::StartInLoop, this));
}

void Connector::StartInLoop() {
  loop_->AssertInLoopThread();
  assert(state_ == kDisconnected);
  if (is_connect_) {
    Connect();
  } else {
    LOG_DEBUG << "do not connect";
  }
}

void Connector::Stop() {
  is_connect_ = false;
  loop_->QueueInLoop(std::bind(&Connector::StopInLoop, this));
}

void Connector::StopInLoop() {
  loop_->AssertInLoopThread();
  if (state_ == kConnecting) {
    SetState(kDisconnected);
    int sockfd = RemoveAndResetChannel();
    Retry(sockfd);
  }
}

void Connector::Connect() {
  int sockfd = socket::CreateNonblockingOrDie(server_addr_.family());
  int ret = socket::Connect(sockfd, server_addr_.GetSockAddr());
  int old_errno = (ret == 0) ? 0 : errno;
  switch (old_errno) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      Connecting(sockfd);
      break;
    case EAGAIN:  // temporary port is exhausted
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      Retry(sockfd);
      break;
    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG_SYSERR << "connect error in Connector::StartInLoop " << old_errno;
      socket::Close(sockfd);
      break;
    default:
      LOG_SYSERR << "Unexpected error in Connector::StartInLoop " << old_errno;
      socket::Close(sockfd);
      break;
  }
}

void Connector::Restart() {
  loop_->AssertInLoopThread();
  SetState(kDisconnected);
  retry_delay_ms_ = kInitRetryDelayMs;
  is_connect_ = true;
  StartInLoop();
}

void Connector::Connecting(int sockfd) {
  SetState(kConnecting);
  assert(!channel_);
  channel_.reset(new Channel(loop_, sockfd));
  channel_->SetWriteCallback(std::bind(&Connector::HandleWrite, this));
  channel_->SetErrorCallback(std::bind(&Connector::HandleError, this));
  channel_->EnableWriting();
}

int Connector::RemoveAndResetChannel() {
  channel_->DisableAll();
  channel_->Remove();
  int sockfd = channel_->fd();
  // Can't reset channel_ here, because we are inside Channel::HandleEvent
  loop_->QueueInLoop(std::bind(&Connector::ResetChannel, this));
  return sockfd;
}

void Connector::ResetChannel() { channel_.reset(); }

void Connector::HandleWrite() {
  LOG_TRACE << "Connector::HandleWrite " << state_;
  if (state_ == kConnecting) {
    int sockfd = RemoveAndResetChannel();
    int err = socket::GetSocketError(sockfd);
    if (err) {
      LOG_WARN << "Connector::HandleWrite - SO_ERROR = " << err << " "
               << strerror_tl(err);
      Retry(sockfd);
    } else if (socket::IsSelfConnect(sockfd)) {
      // ostrich algorithm
      LOG_WARN << "Connector::HandleWrite - Self connect";
      Retry(sockfd);
    } else {
      SetState(kConnected);
      if (is_connect_) {
        new_connection_callback_(sockfd);
      } else {
        // sockfd closed here (after Connector::Stop called)
        socket::Close(sockfd);
      }
    }
  } else {
    assert(state_ == kDisconnected);
  }
}

void Connector::HandleError() {
  LOG_ERROR << "Connector::HandleError state=" << state_;
  if (state_ == kConnecting) {
    int sockfd = RemoveAndResetChannel();
    int err = socket::GetSocketError(sockfd);
    LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
    Retry(sockfd);
  }
}

void Connector::Retry(int sockfd) {
  socket::Close(sockfd);
  SetState(kDisconnected);
  if (is_connect_) {
    LOG_INFO << "Connector::Retry - Retry connecting to "
             << server_addr_.ToIpPort() << " in " << retry_delay_ms_
             << " milliseconds. ";
    // we must use std::shared_ptr to manage Connnector
    loop_->RunAfter(retry_delay_ms_ / 1000.0,
                    std::bind(&Connector::StartInLoop, shared_from_this()));
    retry_delay_ms_ = std::min(retry_delay_ms_ * 2, kMaxRetryDelayMs);
  } else {
    LOG_DEBUG << "do not connect";
  }
}

}  // namespace net

}  // namespace mytinyhttpd