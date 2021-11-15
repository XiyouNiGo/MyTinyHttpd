#include "mytinyhttpd/base/thread.h"

#include <errno.h>
#include <linux/unistd.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>
#include <type_traits>

#include "mytinyhttpd/base/current_thread.h"

namespace mytinyhttpd {

namespace detail {

void AfterFork() {
  CurrentThread::t_cached_tid = 0;
  CurrentThread::t_thread_name = "main";
  CurrentThread::tid();
}

class ThreadNameInitializer {
 public:
  ThreadNameInitializer() {
    CurrentThread::t_thread_name = "main";
    CurrentThread::tid();
    pthread_atfork(NULL, NULL, &AfterFork);
  }
};

ThreadNameInitializer init;

struct ThreadData {
  typedef Thread::ThreadFunc ThreadFunc;
  ThreadFunc func_;
  std::string name_;
  pid_t* tid_;
  CountDownLatch* latch_;

  ThreadData(ThreadFunc func, const std::string& name, pid_t* tid,
             CountDownLatch* latch)
      : func_(std::move(func)), name_(name), tid_(tid), latch_(latch) {}

  void RunInThread() {
    *tid_ = CurrentThread::tid();
    tid_ = NULL;
    latch_->CountDown();
    latch_ = NULL;

    CurrentThread::t_thread_name = name_.empty() ? "Thread" : name_.c_str();
    prctl(PR_SET_NAME, CurrentThread::t_thread_name);
    try {
      func_();
      CurrentThread::t_thread_name = "finished";
    } catch (const std::exception& ex) {
      CurrentThread::t_thread_name = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      abort();
    } catch (...) {
      CurrentThread::t_thread_name = "crashed";
      fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
      throw;  // rethrow
    }
  }
};

void* StartThread(void* obj) {
  ThreadData* data = static_cast<ThreadData*>(obj);
  data->RunInThread();
  delete data;
  return NULL;
}

}  // namespace detail

Thread::Thread(ThreadFunc func, const std::string& n)
    : started_(false),
      joined_(false),
      pthread_id_(0),
      tid_(0),
      func_(std::move(func)),
      name_(n),
      latch_(1) {
  SetDefaultName();
}

Atomic<int32_t> Thread::num_created_;

Thread::~Thread() {
  if (started_ && !joined_) {
    pthread_detach(pthread_id_);
  }
}

void Thread::SetDefaultName() {
  int num = (++num_created_).Load();
  if (name_.empty()) {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread%d", num);
    name_ = buf;
  }
}

void Thread::Start() {
  assert(!started_);
  started_ = true;
  detail::ThreadData* data =
      new detail::ThreadData(func_, name_, &tid_, &latch_);
  if (pthread_create(&pthread_id_, NULL, &detail::StartThread, data) != 0) {
    started_ = false;
    delete data;
    // LOG_SYSFATAL << "Failed in pthread_create";
  } else {
    latch_.Wait();
    assert(tid_ > 0);
  }
}

int Thread::Join() {
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthread_id_, NULL);
}

}  // namespace mytinyhttpd