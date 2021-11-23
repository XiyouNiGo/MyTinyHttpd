#ifndef MYTINYHTTPD_BASE_LOG_STREAM_H_
#define MYTINYHTTPD_BASE_LOG_STREAM_H_

#include <assert.h>
#include <string.h>
#include <strings.h>

#include "mytinyhttpd/utils/noncopyable.h"
#include "mytinyhttpd/utils/slice.h"

namespace mytinyhttpd {

namespace detail {

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template <int SIZE>
class FixedBuffer : public noncopyable {
 public:
  FixedBuffer() : gdb_cookie_(gdb_cookie), cur_(data_) {}

  void Append(const char* buf, size_t len) {
    if (static_cast<size_t>(avail()) > len) {
      ::memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

  const char* data() const { return data_; }
  int length() const { return static_cast<int>(cur_ - data_); }
  char* current() { return cur_; }
  int avail() const { return static_cast<int>(end() - cur_); }

  void Add(size_t len) { cur_ += len; }
  void Reset() { cur_ = data_; }
  void Bzero() { ::bzero(data_, sizeof data_); }

  const char* ToDebugString();

  std::string ToString() const { return std::string(data_, length()); }
  Slice ToSlice() const { return Slice(data_, length()); }

  static void gdb_cookie();

 private:
  const char* end() const { return data_ + sizeof data_; }
  // how to locate data_:
  //  1: info proc mappings
  //  2: find (usually in stack)
  //  3: add sizeof(size_t) to the result of step2
  void (*gdb_cookie_)();
  char data_[SIZE];
  char* cur_;
};

}  // namespace detail

class LogStream : public noncopyable {
  typedef LogStream self;

 public:
  typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;

  self& operator<<(bool v) {
    buffer_.Append(v ? "1" : "0", 1);
    return *this;
  }

  self& operator<<(char v) {
    buffer_.Append(&v, 1);
    return *this;
  }

  self& operator<<(short);
  self& operator<<(unsigned short);
  self& operator<<(int);
  self& operator<<(unsigned int);
  self& operator<<(long);
  self& operator<<(unsigned long);
  self& operator<<(long long);
  self& operator<<(unsigned long long);

  self& operator<<(const void*);

  self& operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
  }
  self& operator<<(double);
  self& operator<<(long double);

  self& operator<<(const char* str) {
    if (str) {
      buffer_.Append(str, strlen(str));
    } else {
      buffer_.Append("(null)", 6);
    }
    return *this;
  }

  self& operator<<(const unsigned char* str) {
    return operator<<(reinterpret_cast<const char*>(str));
  }

  self& operator<<(const std::string& v) {
    buffer_.Append(v.c_str(), v.size());
    return *this;
  }

  self& operator<<(const Slice& v) {
    buffer_.Append(v.data(), v.size());
    return *this;
  }

  self& operator<<(const Buffer& v) {
    *this << v.ToSlice();
    return *this;
  }

  void Append(const char* data, int len) { buffer_.Append(data, len); }
  const Buffer& buffer() const { return buffer_; }
  void ResetBuffer() { buffer_.Reset(); }

 private:
  void StaticCheck();

  template <typename T>
  void FormatInteger(T);

  Buffer buffer_;

  static const int kMaxNumericSize = 48;
};

class Fmt : public noncopyable {
 public:
  template <typename T>
  Fmt(const char* fmt, T val);

  const char* data() const { return buf_; }
  int length() const { return length_; }

 private:
  char buf_[32];
  int length_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt) {
  s.Append(fmt.data(), fmt.length());
  return s;
}

std::string FormatSi(int64_t n);

std::string FormatIec(int64_t n);

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_LOG_STREAM_H_
