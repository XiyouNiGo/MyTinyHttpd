#ifndef MYTINYHTTPD_UTILS_COPYABLE_H_
#define MYTINYHTTPD_UTILS_COPYABLE_H_

namespace mytinyhttpd {

class copyable {
 protected:
  copyable() = default;
  ~copyable() = default;
};

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_UTILS_COPYABLE_H_