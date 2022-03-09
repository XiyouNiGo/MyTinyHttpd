#ifndef MYTINYHTTPD_NET_BUFFER_LIST_H_
#define MYTINYHTTPD_NET_BUFFER_LIST_H_

#include <bits/types/struct_iovec.h>
#include <sys/uio.h>

#include <cstddef>
#include <list>

#include "mytinyhttpd/net/buffer.h"
#include "mytinyhttpd/utils/copyable.h"

namespace mytinyhttpd {

namespace net {

class BufferList : public copyable {
 public:
  static const size_t kInitialSize = 1024;
  explicit BufferList(size_t initial_size = kInitialSize)
      : list_(1, Buffer()) {}
  ~BufferList() = default;

  void swap(BufferList&& x) { std::swap(list_, x.list_); }

  size_t readable_bytes() const {
    size_t rbytes = 0;
    for (auto& buffer : list_) {
      rbytes += buffer.readable_bytes();
    }
    return rbytes;
  }

  size_t writable_bytes() const {
    size_t wbytes = 0;
    for (auto& buffer : list_) {
      wbytes += buffer.writable_bytes();
    }
    return wbytes;
  }

  void Append(const Slice& str) { return back().Append(str); }

  void Append(Buffer&& buffer) { return list_.emplace_back(std::move(buffer)); }

  void Append(const char* data, size_t len) { return back().Append(data, len); }

  void Append(const unsigned char* data, size_t len) {
    return back().Append(data, len);
  }

  void Append(const void* data, size_t len) { return back().Append(data, len); }

  void Append(std::ifstream& is, long len) { return back().Append(is, len); }

  char* OffsetofWrite() { return back().OffsetofWrite(); }

  const char* OffsetofWrite() const { return back().OffsetofWrite(); }

  void AppendInt64(int64_t x) { return back().AppendInt64(x); }

  void AppendInt32(int32_t x) { return back().AppendInt32(x); }

  void AppendInt16(int16_t x) { return back().AppendInt16(x); }

  void AppendInt8(int8_t x) { return back().AppendInt8(x); }

  int64_t ReadInt64() { return front().ReadInt64(); }

  int32_t ReadInt32() { return front().ReadInt32(); }

  int16_t ReadInt16() { return front().ReadInt16(); }

  int8_t ReadInt8() { return front().ReadInt8(); }

  int64_t PeekInt64() const { return front().PeekInt64(); }

  int32_t PeekInt32() const { return front().PeekInt32(); }

  int16_t PeekInt16() const { return front().PeekInt16(); }

  int8_t PeekInt8() const { return front().PeekInt8(); }

  void PrependInt64(int64_t x) { return front().PrependInt64(x); }

  void PrependInt32(int32_t x) { return front().PrependInt32(x); }

  void PrependInt16(int16_t x) { return front().PrependInt16(x); }

  void PrependInt8(int8_t x) { return front().PrependInt8(x); }

  void Prepend(const void* data, size_t len) {
    return front().Prepend(data, len);
  }

  size_t size() { return list_.size(); }

  bool empty() { return list_.empty(); }

  Buffer& front() { return list_.front(); }

  Buffer& front() const { return const_cast<Buffer&>(list_.front()); }

  Buffer& back() { return list_.back(); }

  Buffer& back() const { return const_cast<Buffer&>(list_.back()); }

 private:
  void FillIovec(struct iovec* iov, int iovcnt);

  std::list<Buffer> list_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_BUFFER_LIST_H_
