#include "mytinyhttpd/base/thread_local.h"

#include <gtest/gtest.h>

namespace mytinyhttpd {

TEST(ThreadLocalTest, DereferenceableTest) {
  struct MyType {
    int a;
    int b;
  };

  ThreadLocal<struct MyType> t_mytype;
  (*t_mytype).a = 1;
  (*t_mytype).b = 2;
  ASSERT_EQ((*t_mytype).a, 1);
  ASSERT_EQ((*t_mytype).b, 2);
  ASSERT_EQ(t_mytype->a, 1);
  ASSERT_EQ(t_mytype->b, 2);
}

TEST(ThreadLocalTest, IncompleteTypeTest) {
  class IncompleteType;
  // error: possible problem detected in invocation of delete operator: [-Werror=delete-incomplete]
  // ThreadLocal<IncompleteType> t_inctype;
}

}  // namespace mytinyhttpd