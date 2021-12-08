#ifndef MYTINYHTTPD_NET_BUFFER_H_
#define MYTINYHTTPD_NET_BUFFER_H_

#include <assert.h>
#include <string.h>

#include <algorithm>
#include <boost/container/vector.hpp>
#include <cstdint>
#include <vector>

#include "mytinyhttpd/net/endian.h"
#include "mytinyhttpd/utils/copyable.h"
#include "mytinyhttpd/utils/slice.h"

namespace mytinyhttpd {

namespace net {

class Buffer : public copyable {
 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  explicit Buffer(size_t initial_size = kInitialSize)
      : buffer_(kCheapPrepend + initial_size),
        reader_index_(kCheapPrepend),
        writer_index_(kCheapPrepend) {
    assert(readable_bytes() == 0);
    assert(writable_bytes() == initial_size);
    assert(prependable_bytes() == kCheapPrepend);
  }

  Buffer(Buffer&& x) noexcept { swap(x); }

  void swap(Buffer& x) {
    buffer_.swap(x.buffer_);
    std::swap(reader_index_, x.reader_index_);
    std::swap(writer_index_, x.writer_index_);
  }

  size_t readable_bytes() const { return writer_index_ - reader_index_; }

  size_t writable_bytes() const { return buffer_.size() - writer_index_; }

  size_t prependable_bytes() const { return reader_index_; }

  const char* Peek() const { return begin() + reader_index_; }

  const char* FindCrlf() const {
    const char* crlf = std::search(Peek(), OffsetofWrite(), kCrlf, kCrlf + 2);
    return crlf == OffsetofWrite() ? nullptr : crlf;
  }

  const char* FindCrlf(const char* start) const {
    assert(Peek() <= start);
    assert(start <= OffsetofWrite());
    const char* crlf = std::search(start, OffsetofWrite(), kCrlf, kCrlf + 2);
    return crlf == OffsetofWrite() ? nullptr : crlf;
  }

  const char* FindEol() const {
    const void* eol = memchr(Peek(), '\n', readable_bytes());
    return static_cast<const char*>(eol);
  }

  const char* FindEol(const char* start) const {
    assert(Peek() <= start);
    assert(start <= OffsetofWrite());
    const void* eol = memchr(start, '\n', OffsetofWrite() - start);
    return static_cast<const char*>(eol);
  }

  void Retrieve(size_t len) {
    assert(len <= readable_bytes());
    if (len < readable_bytes()) {
      reader_index_ += len;
    } else {
      RetrieveAll();
    }
  }

  void RetrieveUntil(const char* end) {
    assert(Peek() <= end);
    assert(end <= OffsetofWrite());
    Retrieve(end - Peek());
  }

  void RetrieveInt64() { Retrieve(sizeof(int64_t)); }

  void RetrieveInt32() { Retrieve(sizeof(int32_t)); }

  void RetrieveInt16() { Retrieve(sizeof(int16_t)); }

  void RetrieveInt8() { Retrieve(sizeof(int8_t)); }

  void RetrieveAll() {
    // spurcious retrieve
    reader_index_ = kCheapPrepend;
    writer_index_ = kCheapPrepend;
  }

  std::string RetrieveAllAsString() {
    return RetrieveAsString(readable_bytes());
  }

  std::string RetrieveAsString(size_t len) {
    assert(len <= readable_bytes());
    std::string result(Peek(), len);
    Retrieve(len);
    return result;
  }

  Slice ToSlice() const {
    return Slice(Peek(), static_cast<int>(readable_bytes()));
  }

  void Append(const Slice& str) { Append(str.data(), str.size()); }

  void Append(const char* data, size_t len) {
    EnsureWritableBytes(len);
    std::copy(data, data + len, OffsetofWrite());
    HasWritten(len);
  }

  void Append(const unsigned char* data, size_t len) {
    EnsureWritableBytes(len);
    std::copy(data, data + len, OffsetofWrite());
    HasWritten(len);
  }

  void Append(const void* data, size_t len) {
    Append(static_cast<const char*>(data), len);
  }

  void EnsureWritableBytes(size_t len) {
    if (writable_bytes() < len) {
      MakeSpace(len);
    }
    assert(writable_bytes() >= len);
  }

  char* OffsetofWrite() { return begin() + writer_index_; }

  const char* OffsetofWrite() const { return begin() + writer_index_; }

  void HasWritten(size_t len) {
    assert(len <= writable_bytes());
    writer_index_ += len;
  }

  void Unwrite(size_t len) {
    assert(len <= readable_bytes());
    writer_index_ -= len;
  }

  void AppendInt64(int64_t x) {
    int64_t be64 = static_cast<int64_t>(socket::HostToNetwork64(x));
    Append(&be64, sizeof be64);
  }

  void AppendInt32(int32_t x) {
    int32_t be32 = static_cast<int32_t>(socket::HostToNetwork32(x));
    Append(&be32, sizeof be32);
  }

  void AppendInt16(int16_t x) {
    int16_t be16 = static_cast<int16_t>(socket::HostToNetwork16(x));
    Append(&be16, sizeof be16);
  }

  void AppendInt8(int8_t x) { Append(&x, sizeof x); }

  int64_t ReadInt64() {
    int64_t result = PeekInt64();
    RetrieveInt64();
    return result;
  }

  int32_t ReadInt32() {
    int32_t result = PeekInt32();
    RetrieveInt32();
    return result;
  }

  int16_t ReadInt16() {
    int16_t result = PeekInt16();
    RetrieveInt16();
    return result;
  }

  int8_t ReadInt8() {
    int8_t result = PeekInt8();
    RetrieveInt8();
    return result;
  }

  int64_t PeekInt64() const {
    assert(readable_bytes() >= sizeof(int64_t));
    int64_t be64 = 0;
    ::memcpy(&be64, Peek(), sizeof be64);
    return static_cast<int64_t>(socket::NetworkToHost64(be64));
  }

  int32_t PeekInt32() const {
    assert(readable_bytes() >= sizeof(int32_t));
    int32_t be32 = 0;
    ::memcpy(&be32, Peek(), sizeof be32);
    return static_cast<int32_t>(socket::NetworkToHost32(be32));
  }

  int16_t PeekInt16() const {
    assert(readable_bytes() >= sizeof(int16_t));
    int16_t be16 = 0;
    ::memcpy(&be16, Peek(), sizeof be16);
    return static_cast<int16_t>(socket::NetworkToHost16(be16));
  }

  int8_t PeekInt8() const {
    assert(readable_bytes() >= sizeof(int8_t));
    int8_t x = *Peek();
    return x;
  }

  void PrependInt64(int64_t x) {
    int64_t be64 = static_cast<int64_t>(socket::HostToNetwork64(x));
    Prepend(&be64, sizeof be64);
  }

  void PrependInt32(int32_t x) {
    int32_t be32 = static_cast<int32_t>(socket::HostToNetwork32(x));
    Prepend(&be32, sizeof be32);
  }

  void PrependInt16(int16_t x) {
    int16_t be16 = static_cast<int16_t>(socket::HostToNetwork16(x));
    Prepend(&be16, sizeof be16);
  }

  void PrependInt8(int8_t x) { Prepend(&x, sizeof x); }

  void Prepend(const void* data, size_t len) {
    assert(len <= prependable_bytes());
    reader_index_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d + len, begin() + reader_index_);
  }

  void Shrink(size_t reserve) {
    Buffer temp;
    temp.EnsureWritableBytes(readable_bytes() + reserve);
    temp.Append(ToSlice());
    temp.buffer_.shrink_to_fit();
    swap(temp);
  }

  size_t capacity() const { return buffer_.capacity(); }

  ssize_t ReadFromFd(int fd, int* old_errno);

 private:
  char* begin() { return &*buffer_.begin(); }

  const char* begin() const { return &*buffer_.begin(); }

  void MakeSpace(size_t len);

  void MoveReadableFront(Buffer& dst);

  void MoveReadaleFront() { MoveReadableFront(*this); }

 private:
  boost::container::vector<char>
      buffer_;  // TODO: change the buffer type to user-defined to
                // keep away to valid intialization (during resize())
  size_t reader_index_;
  size_t writer_index_;

  static const char* kCrlf;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_BUFFER_H_
