#include "mytinyhttpd/utils/slice.h"

#include <gtest/gtest.h>

#include <cstring>

namespace mytinyhttpd {

const char *s1 = "I'm a const char *";
const std::string s2 = "I'm a const std::string";

TEST(MyTinyHttpdTest, ConstructorTest) {
  Slice slice1;
  Slice slice2(s1);
  Slice slice3(s1, 5);
  Slice slice4(s2);
}

TEST(MyTinyHttpdTest, MemberFuncTest) {
  {
    Slice slice(s1);
    ASSERT_EQ(slice.data(), s1);
    ASSERT_EQ(slice.size(), strlen(s1));
    ASSERT_EQ(slice[5], s1[5]);
  }
  {
    Slice slice(s1);
    slice.clear();
    ASSERT_EQ(slice.data(), "");
    ASSERT_EQ(slice.size(), 0U);
  }
  {
    Slice slice(s1);
    Slice prefix("I'm a");
    ASSERT_TRUE(slice.starts_with(prefix));
    Slice not_prefix("Not a prefix for slice");
    ASSERT_FALSE(slice.starts_with(not_prefix));
  }
  {
    Slice slice(s1);
    ASSERT_EQ(slice.data(), s1);
    slice.remove_prefix(5);
    ASSERT_EQ(slice.data(), s1 + 5);
  }
}

}  // namespace mytinyhttpd