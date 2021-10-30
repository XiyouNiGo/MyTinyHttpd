#ifndef MYTINYHTTPD_UTILS_NONCOPYABLE_H_
#define MYTINYHTTPD_UTILS_NONCOPYABLE_H_

namespace mytinyhttpd {

class noncopyable {
 public:
  noncopyable(const noncopyable&) = delete;
  void operator=(const noncopyable&) = delete;

 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_UTILS_NONCOPYABLE_H_