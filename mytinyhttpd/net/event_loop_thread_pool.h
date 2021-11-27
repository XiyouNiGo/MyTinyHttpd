#ifndef MYTINYHTTPD_NET_EVENT_LOOP_THREAD_POOL_H_
#define MYTINYHTTPD_NET_EVENT_LOOP_THREAD_POOL_H_

#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

#include "mytinyhttpd/utils/noncopyable.h"

namespace mytinyhttpd {

namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : public noncopyable {
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;

  EventLoopThreadPool(EventLoop* base_loop, const std::string& name_arg);
  ~EventLoopThreadPool() = default;

  void SetThreadNum(int num_threads) { num_threads_ = num_threads; }
  void Start(const ThreadInitCallback& init_callback = ThreadInitCallback());
  void ResizeThreadNum(
      size_t num_threads,
      const ThreadInitCallback& init_callback = ThreadInitCallback());

  // valid after calling start()
  /// round-robin
  EventLoop* GetNextLoop();

  EventLoop* GetLoopByHash(size_t hash_code);

  std::vector<EventLoop*> GetAllLoops();

  bool IsStarted() const { return is_started_; }

  const std::string& name() const { return name_; }

 private:
  EventLoop* base_loop_;  // stack variable
  std::string name_;
  bool is_started_;
  size_t num_threads_;
  int next_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_EVENT_LOOP_THREAD_POOL_H_
