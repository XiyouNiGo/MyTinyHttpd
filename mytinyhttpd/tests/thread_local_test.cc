#include "mytinyhttpd/base/thread_local.h"

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

TEST(ThreadLocalTest, DereferenceableTest) {
  ThreadLocal<struct Data> t_data;
  (*t_data).data = "data";
  ASSERT_STREQ((*t_data).data, "data");
  ASSERT_STREQ(t_data->data, "data");
}

TEST(ThreadLocalTest, ThreadLocalTest) {
  ThreadLocal<struct Data> t_data1;
  (*t_data1).data = "data";
  ASSERT_STREQ((*t_data1).data, "data");
  ASSERT_STREQ(t_data1->data, "data");
  Thread thread([]() {
    ThreadLocal<struct Data> t_data2;
    (*t_data2).data = "data";
    ASSERT_STREQ((*t_data2).data, "data");
    ASSERT_STREQ(t_data2->data, "data");
  });
  thread.Start();
  thread.Join();
}

TEST(ThreadLocalTest, IncompleteTypeTest) {
  class IncompleteType;
  // error: possible problem detected in invocation of delete operator:
  // [-Werror=delete-incomplete] ThreadLocal<IncompleteType> t_inctype;
}