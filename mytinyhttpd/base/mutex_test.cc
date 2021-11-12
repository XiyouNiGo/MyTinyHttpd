#include "mytinyhttpd/base/mutex.h"

#include <gtest/gtest.h>

namespace mytinyhttpd {

const int kAddTimesPerThread = 100000;
const int kNumThreads = 10;
const int kAddTimesTotal = kAddTimesPerThread * kNumThreads;

MutexLock mutex;
int count GUARDED_BY(mutex) = 0;

void *ThreadFunc(void *arg) {
  MutexLockGuard guard(mutex);
  for (int i = 0; i < kAddTimesPerThread; i++) {
    count++;
  }
  return nullptr;
}

TEST(MutexTest, SharedAddTest) {
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

  ASSERT_EQ(count, kAddTimesTotal);
}

}  // namespace mytinyhttpd