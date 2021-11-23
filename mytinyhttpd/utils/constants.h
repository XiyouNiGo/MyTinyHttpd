#ifndef MYTINYHTTPD_UTILS_CONSTANTS_H_
#define MYTINYHTTPD_UTILS_CONSTANTS_H_

#include <stdio.h>
#include <stdlib.h>

#include <cstdint>

#define BIT_NUM_PER_BYTE 8

#define DEFAULT_NAME "unknown"
#define DEFAULT_NAME_LENGTH 7

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define HandleErr(err_msg)                                      \
  do {                                                          \
    fprintf(stderr, "(File:%s, Line:%d) ", __FILE__, __LINE__); \
    perror(err_msg);                                            \
    exit(-1);                                                   \
  } while (0)

#define CheckRetVal(expression, right_val, err_msg) \
  do {                                              \
    if (unlikely(expression != right_val)) {        \
      HandleErr(err_msg);                           \
    }                                               \
  } while (0)

const int64_t kBytesPerKilobyte = 1024;
const int64_t kKilobytesPerMegabyte = 1024;
const int64_t kMegabytesPerGigabyte = 1024;
const int64_t kBytesPerMegabyte = 1024 * 1024;
const int64_t kBytesPerGigabyte = 1024 * 1024 * 1024;
const int64_t kKilobytesPerGigabyte = 1024 * 1024;

#endif  // !MYTINYHTTPD_UTILS_CONSTANTS_H_