#ifndef MYTINYHTTPD_UTILS_SLICE_H_
#define MYTINYHTTPD_UTILS_SLICE_H_

#include <assert.h>
#include <string.h>

#include <string>

#include "mytinyhttpd/utils/copyable.h"

namespace mytinyhttpd {

class Slice : copyable {
 public:
  Slice() : data_(""), size_(0) {}

  Slice(const char* data, size_t size) : data_(data), size_(size) {}

  Slice(const std::string& s) : data_(s.data()), size_(s.size()) {}

  Slice(const char* s) : data_(s), size_(strlen(s)) {}

  const char* data() const { return data_; }

  size_t size() const { return size_; }

  bool empty() const { return size_ == 0; }

  char operator[](size_t n) const {
    assert(n < size());
    return data_[n];
  }

  void clear() {
    data_ = "";
    size_ = 0;
  }

  void remove_prefix(size_t n) {
    assert(n <= size());
    data_ += n;
    size_ -= n;
  }

  void remove_suffix(size_t n) {
    assert(n <= size());
    size_ -= n;
  }

  std::string ToString() const { return std::string(data_, size_); }

  int compare(const Slice& slice) const;

  bool starts_with(const Slice& slice) const {
    return ((size_ >= slice.size_) &&
            (memcmp(data_, slice.data_, slice.size_) == 0));
  }

 private:
  const char* data_;
  size_t size_;
};

inline bool operator==(const Slice& lhs, const Slice& rhs) {
  return ((lhs.size() == rhs.size()) &&
          (memcmp(lhs.data(), rhs.data(), lhs.size()) == 0));
}

inline bool operator!=(const Slice& lhs, const Slice& rhs) {
  return !(lhs == rhs);
}

inline int Slice::compare(const Slice& slice) const {
  const size_t min_len = (size_ < slice.size_) ? size_ : slice.size_;
  int r = memcmp(data_, slice.data_, min_len);
  if (r == 0) {
    if (size_ < slice.size_)
      r = -1;
    else if (size_ > slice.size_)
      r = +1;
  }
  return r;
}

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_UTILS_SLICE_H_