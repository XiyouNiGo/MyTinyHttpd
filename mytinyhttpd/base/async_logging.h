#ifndef MYTINYHTTPD_BASE_ASYNC_LOGGING_H_
#define MYTINYHTTPD_BASE_ASYNC_LOGGING_H_

#include <atomic>
#include <vector>

#include "mytinyhttpd/base/count_down_latch.h"
#include "mytinyhttpd/base/log_stream.h"
#include "mytinyhttpd/base/mutex.h"
#include "mytinyhttpd/base/thread.h"

namespace mytinyhttpd {

class AsyncLogging : noncopyable {
 public:
  AsyncLogging(const std::string& basename, off_t roll_size,
               int flush_interval = 3);

  ~AsyncLogging() {
    if (is_running_) {
      Stop();
    }
  }

  void Append(const char* logline, int len);

  void Start() {
    is_running_ = true;
    thread_.Start();
    latch_.Wait();
  }

  void Stop() {
    is_running_ = false;
    cond_.Notify();
    thread_.Join();
  }

 private:
  void ThreadFunc();

  typedef detail::FixedBuffer<detail::kLargeBuffer> Buffer;
  typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
  typedef BufferVector::value_type BufferPtr;

  const int flush_interval_;
  std::atomic<bool> is_running_;
  const std::string basename_;
  const off_t roll_size_;
  Thread thread_;
  CountDownLatch latch_;
  MutexLock mutex_;
  Condition cond_ GUARDED_BY(mutex_);
  // four log buffer
  BufferPtr current_buffer_ GUARDED_BY(mutex_);
  BufferPtr next_buffer_ GUARDED_BY(mutex_);
  BufferVector buffers_ GUARDED_BY(mutex_);
};

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_ASYNC_LOGGING_H_
