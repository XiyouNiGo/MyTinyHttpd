#include "mytinyhttpd/net/buffer_list.h"

namespace mytinyhttpd {

namespace net {

void BufferList::FillIovec(struct iovec* iov, int iovcnt) {
  assert(iovcnt <= static_cast<int>(size()));
  int i = 0;
  for (auto& buffer : list_) {
    iov[i].iov_base = const_cast<char*>(buffer.Peek());
    iov[i].iov_len = buffer.readable_bytes();
    i++;
  }
}

}  // namespace net

}  // namespace mytinyhttpd