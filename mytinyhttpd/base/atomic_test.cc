#include "mytinyhttpd/base/atomic.h"

#include <gtest/gtest.h>

namespace mytinyhttpd {

const int kAddTimesPerThread = 100000;
const int kNumThreads = 10;
const int kAddTimesTotal = kAddTimesPerThread * kNumThreads;

Atomic<int> count(0);

void *ThreadFunc(void *arg) {
  for (int i = 0; i < kAddTimesPerThread; i++) {
    count++;
  }
  return nullptr;
}

TEST(ThreadPoolTest, SharedAddTest) {
  pthread_t threads[kNumThreads] = {0};
  int ret = 0;

  for (int i = 0; i < kNumThreads; i++) {
    ret = pthread_create(&threads[i], nullptr, ThreadFunc, nullptr);
    if (ret != 0) {
      perror("pthread_create error");
    }
  }
  for (int i = 0; i < kNumThreads; i++) {
    pthread_join(threads[i], nullptr);
  }

  ASSERT_EQ(count.Load(), kAddTimesTotal);
}

}  // namespace mytinyhttpd