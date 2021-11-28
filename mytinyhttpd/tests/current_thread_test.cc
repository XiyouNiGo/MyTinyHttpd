#include "mytinyhttpd/base/current_thread.h"

#include <gtest/gtest.h>

namespace mytinyhttpd {

namespace CurrentThread {

TEST(CurrentThreadTest, CacheTidTest) {
  ASSERT_EQ(t_cached_tid, 0);
  CacheTid();
  ASSERT_GT(t_cached_tid, 0);
  pid_t temp_pid = t_cached_tid;
  CacheTid();
  ASSERT_EQ(t_cached_tid, temp_pid);
}

}  // namespace CurrentThread

}  // namespace mytinyhttpd