#include "mytinyhttpd/net/buffer.h"

#include <errno.h>
#include <sys/uio.h>

#include "mytinyhttpd/net/socket_ops.h"

namespace mytinyhttpd {

namespace net {

const char* Buffer::kCrlf = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

ssize_t Buffer::ReadFromFd(int fd, int* old_errno) {
  char extrabuf[65536];  // buffer on the stack
  struct iovec vec[2];
  const size_t writable = writable_bytes();
  vec[0].iov_base = begin() + writer_index_;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;

  const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
  const ssize_t n = socket::Readv(fd, vec, iovcnt);
  if (n < 0) {
    *old_errno = errno;
  } else if (static_cast<size_t>(n) <= writable) {
    writer_index_ += n;
  } else {
    writer_index_ = buffer_.size();
    Append(extrabuf, n - writable);
  }
  // we do not call read repeatedly to reduce latency in message processing
  return n;
}

void Buffer::MakeSpace(size_t len) {
  assert(writable_bytes() < len);
  if (writable_bytes() + prependable_bytes() < len + kCheapPrepend) {
    // if capacity is adequate
    if (writer_index_ + len < capacity()) {
      // avoid resize that sets tail elements to 0
      buffer_.resize(writer_index_ + len, boost::container::default_init);
    } else {
      // move readable data to a new Buffer then swap
      Buffer temp(writer_index_ + len);
      // assert(temp.readable_bytes() == 0);
      MoveReadableFront(temp);
      swap(temp);
    }
  } else {
    // move readable data to the front
    MoveReadaleFront();
  }
}

void Buffer::MoveReadableFront(Buffer& dst) {
  assert(kCheapPrepend <= reader_index_);
  size_t readable = readable_bytes();
  std::copy(begin() + reader_index_, begin() + writer_index_,
            dst.begin() + kCheapPrepend);
  dst.reader_index_ = kCheapPrepend;
  dst.writer_index_ = dst.reader_index_ + readable;
  assert(readable == dst.readable_bytes());
}

}  // namespace net

}  // namespace mytinyhttpd
