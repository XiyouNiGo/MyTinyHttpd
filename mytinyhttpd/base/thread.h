#ifndef MYTINYHTTPD_BASE_THREAD_H_
#define MYTINYHTTPD_BASE_THREAD_H_

#include <errno.h>
#include <linux/unistd.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

namespace mytinyhttpd {

namespace detail {

inline pid_t gettid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }

}  // namespace detail

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_THREAD_H_