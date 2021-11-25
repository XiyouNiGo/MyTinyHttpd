#include "mytinyhttpd/net/acceptor.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/net/event_loop.h"
#include "mytinyhttpd/net/inet_address.h"
#include "mytinyhttpd/net/socket_ops.h"

namespace mytinyhttpd {

namespace net {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listen_addr,
                   bool is_reuseport)
    : loop_(loop),
      accept_socket_(socket::CreateNonblockingOrDie(listen_addr.family())),
      accept_channel_(loop, accept_socket_.socketfd()),
      is_listening_(false),
      idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
  assert(idle_fd_ >= 0);
  accept_socket_.SetReuseAddr(true);
  accept_socket_.SetReusePort(is_reuseport);
  accept_socket_.BindAddress(listen_addr);
  accept_channel_.SetReadCallback(std::bind(&Acceptor::HandleRead, this));
}

Acceptor::~Acceptor() {
  accept_channel_.DisableAll();
  accept_channel_.Remove();
  ::close(idle_fd_);
}

void Acceptor::Listen() {
  loop_->AssertInLoopThread();
  is_listening_ = true;
  accept_socket_.Listen();
  accept_channel_.EnableReading();
}

void Acceptor::HandleRead() {
  loop_->AssertInLoopThread();
  InetAddress peer_addr;
  int connfd = accept_socket_.Accept(&peer_addr);
  if (connfd >= 0) {
    if (new_connection_callback_) {
      new_connection_callback_(connfd, peer_addr);
    } else {
      socket::Close(connfd);
    }
  } else {
    LOG_SYSERR << "in Acceptor::handleRead";
    if (errno == EMFILE) {
      ::close(idle_fd_);
      idle_fd_ = ::accept(accept_socket_.socketfd(), NULL, NULL);
      ::close(idle_fd_);
      idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    }
  }
}

}  // namespace net

}  // namespace mytinyhttpd