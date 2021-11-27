#include "mytinyhttpd/base/thread_pool.h"

#include <assert.h>
#include <stdio.h>

#include <cstddef>
#include <utility>

namespace mytinyhttpd {

ThreadPool::ThreadPool(std::string name_arg, size_t max_queue_size,
                       const Task& thread_init_callback)
    : mutex_(),
      not_empty_(mutex_),
      not_full_(mutex_),
      name_(std::move(name_arg)),
      thread_init_callback_(thread_init_callback),
      max_queue_size_(max_queue_size),
      running_(false) {}

ThreadPool::~ThreadPool() {
  if (running_) {
    Stop();
  }
}

void ThreadPool::Start(size_t num_threads) {
  assert(threads_.empty());
  running_ = true;
  threads_.reserve(num_threads);
  for (size_t i = 0; i < num_threads; ++i) {
    char buf[name_.size() + 32];
    snprintf(buf, sizeof buf, "%s%lu", name_.c_str(), i);
    threads_.emplace_back(
        new Thread(std::bind(&ThreadPool::RunInThread, this), buf));
    threads_[i]->Start();
  }
  // use main thread as worker thread
  if (num_threads == 0 && thread_init_callback_) {
    thread_init_callback_();
  }
}

void ThreadPool::Stop() {
  {
    // delay to stop
    MutexLockGuard lock(mutex_);
    running_ = false;
    not_empty_.NotifyAll();
    not_full_.NotifyAll();
  }
  for (auto& t : threads_) {
    t->Join();
  }
}

size_t ThreadPool::queue_size() const {
  MutexLockGuard lock(mutex_);
  return queue_.size();
}

void ThreadPool::Run(Task task) {
  if (threads_.empty()) {
    task();
  } else {
    MutexLockGuard lock(mutex_);
    while (running_ && IsFull()) {
      not_full_.Wait();
    }
    if (!running_) return;
    assert(!IsFull());
    queue_.push_back(std::move(task));
    not_empty_.Notify();
  }
}

void ThreadPool::ResizeThreadNum(size_t num_threads) {
  assert(running_);
  size_t old_num_threads = threads_.size();
  if (num_threads < old_num_threads) {
    threads_.resize(num_threads);
    threads_.shrink_to_fit();
  } else if (num_threads > old_num_threads) {
    threads_.reserve(num_threads);
    for (size_t i = old_num_threads; i < num_threads; ++i) {
      char buf[name_.size() + 32];
      snprintf(buf, sizeof buf, "%s%lu", name_.c_str(), i);
      threads_.emplace_back(
          new Thread(std::bind(&ThreadPool::RunInThread, this), buf));
      threads_[i]->Start();
    }
  }
  if (num_threads == 0 && thread_init_callback_) {
    thread_init_callback_();
  }
}

ThreadPool::Task ThreadPool::Pop() {
  MutexLockGuard lock(mutex_);
  while (queue_.empty() && running_) {
    not_empty_.Wait();
  }
  Task task;
  if (!queue_.empty()) {
    task = queue_.front();
    queue_.pop_front();
    if (max_queue_size_ > 0) {
      not_full_.Notify();
    }
  }
  return task;
}

bool ThreadPool::IsFull() const {
  mutex_.AssertLocked();
  return max_queue_size_ > 0 && queue_.size() >= max_queue_size_;
}

void ThreadPool::RunInThread() {
  try {
    if (thread_init_callback_) {
      thread_init_callback_();
    }
    while (running_) {
      Task task(Pop());
      if (task) {
        task();
      }
    }
  } catch (const std::exception& ex) {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  } catch (...) {
    fprintf(stderr, "unknown exception caught in ThreadPool %s\n",
            name_.c_str());
    throw;  // rethrow
  }
}

}  // namespace mytinyhttpd