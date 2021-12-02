#include "mytinyhttpd/base/singleton.h"

#include <gtest/gtest.h>

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/base/mutex.h"
#include "mytinyhttpd/base/thread.h"
#include "mytinyhttpd/utils/noncopyable.h"

using namespace mytinyhttpd;

struct Data : public noncopyable {
  Data() {
    LOG_INFO << "tid = " << CurrentThread::tid() << ", Data constructs in "
             << this;
  }

  ~Data() {
    LOG_INFO << "tid = " << CurrentThread::tid() << ", Data destructs in "
             << this;
  }

  const char *data = "nullptr";
};

TEST(SingletonTest, SingletonTest) {
  struct Data *data1 = &Singleton<Data>::Instance();
  struct Data *data2 = &Singleton<Data>::Instance();
  ASSERT_STREQ(data1->data, "nullptr");
  ASSERT_STREQ(data2->data, "nullptr");
  ASSERT_EQ(data1, data2);
  data1->data = "data1";
  ASSERT_STREQ(data2->data, "data1");
  data2->data = "data2";
  ASSERT_STREQ(data1->data, "data2");
}

TEST(SingletonTest, ThreadLocalTest) {
  struct Data *data1 = &Singleton<Data>::Instance();
  data1->data = "data1";
  Thread thread([]() {
    struct Data *data2 = &Singleton<Data>::Instance();
    ASSERT_STREQ(data2->data, "data1");
    data2->data = "data2";
  });
  thread.Start();
  thread.Join();
  ASSERT_STREQ(data1->data, "data2");
}