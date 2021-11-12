#ifndef MYTINYHTTPD_BASE_THREAD_POOL_H_
#define MYTINYHTTPD_BASE_THREAD_POOL_H_

#include <cstddef>
#include <deque>
#include <string>
#include <thread>
#include <vector>

#include "mytinyhttpd/base/condition.h"
#include "mytinyhttpd/base/mutex.h"
#include "mytinyhttpd/base/thread.h"

namespace mytinyhttpd {

class ThreadPool : noncopyable {
 public:
  typedef std::function<void()> Task;

  explicit ThreadPool(
      std::string name_arg = "ThreadPool",
      size_t max_queue_size = 0, /* 0 on behalf of unlimited size */
      const Task& thread_init_callback_ = nullptr);
  ~ThreadPool();

  // must be called before Start()
  void SetMaxQueueSize(int max_size) { max_queue_size_ = max_size; }
  void SetThreadInitCallback(const Task& cb) { thread_init_callback_ = cb; }

  void Start(size_t num_threads = std::thread::hardware_concurrency());
  void Stop();

  std::string name() const { return name_; }

  size_t queue_size() const;

  void Run(Task f);

  // must be called after Start()
  void ResizeThreadNum(size_t num_threads);
  size_t thread_num() const { return threads_.size(); }

 private:
  bool IsFull() const REQUIRES(mutex_);
  void RunInThread();
  Task Pop();

  mutable MutexLock mutex_;
  Condition not_empty_ GUARDED_BY(mutex_);
  Condition not_full_ GUARDED_BY(mutex_);
  std::string name_;
  Task thread_init_callback_;
  // we must use a pointer because mytinyhttpd::Thread is noncopyable
  std::vector<std::unique_ptr<Thread>> threads_;
  std::deque<Task> queue_ GUARDED_BY(mutex_);
  size_t max_queue_size_;
  bool running_;
};

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_THREAD_POOL_H_
