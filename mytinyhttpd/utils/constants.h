#ifndef MYTINYHTTPD_UTILS_CONSTANTS_H_
#define MYTINYHTTPD_UTILS_CONSTANTS_H_

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#endif  // !MYTINYHTTPD_UTILS_CONSTANTS_H_