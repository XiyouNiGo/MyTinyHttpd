#include "mytinyhttpd/base/thread_pool.h"

#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <thread>

#include "mytinyhttpd/base/atomic.h"
#include "mytinyhttpd/base/thread.h"

namespace mytinyhttpd {

TEST(ThreadPoolTest, MemverVarTest) {
  ThreadPool pool;
  ASSERT_STREQ(pool.name().c_str(), "ThreadPool");
  ASSERT_EQ(pool.queue_size(), 0UL);
}

TEST(ThreadPoolTest, RuntimesTest) {
  {
    Atomic<int> count(0);
    int run_times = 100000;
    {
      ThreadPool pool;
      for (int i = 0; i < run_times; i++) {
        pool.Run([&] { count++; });
      }
    }
    ASSERT_EQ(count.Load(), run_times);
  }
  // FIXME: probabilistic failure
  {
    Atomic<int> count(0);
    int run_times = 100000;
    {
      ThreadPool pool;
      pool.Start();
      ASSERT_EQ(std::thread::hardware_concurrency(),
                static_cast<size_t>(Thread::num_created()));
      for (int i = 0; i < run_times; i++) {
        pool.Run([&] { count++; });
      }

      pool.ResizeThreadNum(std::thread::hardware_concurrency() * 2);
      ASSERT_EQ(std::thread::hardware_concurrency() * 2,
                static_cast<size_t>(Thread::num_created()));
      for (int i = 0; i < run_times; i++) {
        pool.Run([&] { count++; });
      }
    }
    ASSERT_EQ(count.Load(), run_times * 2);
  }
  {
    Atomic<int> count(0);
    int run_times = 100000;
    {
      ThreadPool pool;
      pool.Start();
      for (int i = 0; i < run_times; i++) {
        pool.Run([&] { count++; });
      }
    }
    ASSERT_EQ(count.Load(), run_times);
  }
}

TEST(ThreadPoolTest, ThreadNumTest) {
  ThreadPool pool;
  ASSERT_EQ(pool.thread_num(), 0UL);
  pool.Start();
  ASSERT_EQ(pool.thread_num(), std::thread::hardware_concurrency());
  pool.ResizeThreadNum(std::thread::hardware_concurrency() / 2);
  ASSERT_EQ(pool.thread_num(), std::thread::hardware_concurrency() / 2);
  pool.ResizeThreadNum(std::thread::hardware_concurrency() * 2);
  ASSERT_EQ(pool.thread_num(), std::thread::hardware_concurrency() * 2);
}

}  // namespace mytinyhttpd