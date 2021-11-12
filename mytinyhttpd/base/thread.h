#ifndef MYTINYHTTPD_BASE_THREAD_H_
#define MYTINYHTTPD_BASE_THREAD_H_

#include <errno.h>
#include <linux/unistd.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <functional>
#include <memory>

#include "mytinyhttpd/base/atomic.h"
#include "mytinyhttpd/base/count_down_latch.h"
#include "mytinyhttpd/utils/noncopyable.h"

namespace mytinyhttpd {

namespace detail {

inline pid_t gettid() { return static_cast<pid_t>(syscall(SYS_gettid)); }

}  // namespace detail

class CountDownLatch;

class Thread : public noncopyable {
 public:
  typedef std::function<void()> ThreadFunc;

  explicit Thread(ThreadFunc, const std::string& name = std::string());
  ~Thread();

  void Start();
  bool IsStarted() const { return started_; }

  int Join();

  pid_t tid() const { return tid_; }
  const std::string& name() const { return name_; }
  static int num_created() { return num_created_.Load(); }

 private:
  void SetDefaultName();

  bool started_;
  bool joined_;
  pthread_t pthread_id_;  // only exposed to Pthread Library
  pid_t tid_;
  ThreadFunc func_;
  std::string name_;
  CountDownLatch latch_;
  static Atomic<int32_t> num_created_;
};

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_THREAD_H_