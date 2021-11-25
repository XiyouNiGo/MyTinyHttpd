#include "mytinyhttpd/net/timer_queue.h"

#include <gtest/gtest.h>

#include "mytinyhttpd/net/channel.h"
#include "mytinyhttpd/net/event_loop.h"
#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

namespace net {

class TimerQueueTf : public testing::Test {
 public:
  void SetUp() {
    printf("pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
    printf("now: %s\n", Timestamp::Now().ToFormattedString().c_str());
    remain_run_times = kRunMaxTimes;
  }

  void TearDown() { loop.Loop(); }

  void Print(const char* msg) {
    printf("%s: %s\n", msg, Timestamp::Now().ToFormattedString().c_str());
    if (remain_run_times-- <= 0) {
      loop.Quit();
    }
  }

  EventLoop loop;

  static const int kRunMaxTimes = 20;
  int remain_run_times;
};

TEST_F(TimerQueueTf, RunAfterTest) {
  for (int i = 0; i < 10; i++) {
    loop.RunAfter(
        0.5, std::bind(&TimerQueueTf::Print, this, "EvenLoop::RunAfter 0.5"));
    loop.RunAfter(
        0.75, std::bind(&TimerQueueTf::Print, this, "EvenLoop::RunAfter 0.75"));
    loop.RunAfter(
        1, std::bind(&TimerQueueTf::Print, this, "EvenLoop::RunAfter 1"));
  }
}

TEST_F(TimerQueueTf, RunEveryTest) {
  loop.RunEvery(
      0.5, std::bind(&TimerQueueTf::Print, this, "EvenLoop::RunEvery 0.5"));
}

}  // namespace net

}  // namespace mytinyhttpd