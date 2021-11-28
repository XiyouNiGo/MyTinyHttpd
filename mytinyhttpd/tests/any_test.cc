#include "mytinyhttpd/utils/any.h"

#include <gtest/gtest.h>

#include <string>
#include <typeinfo>

namespace mytinyhttpd {

TEST(AnyTest, ConstructorTest) {
  { Any a; }
  {
    Any other;
    Any a(other);
  }
  {
    const Any other;
    Any a(other);
  }
  { Any a((Any())); }
  { Any a(static_cast<const Any>((Any()))); }
  {
    Any ai(1);
    Any ac('c');
    Any as((std::string()));
  }
}

TEST(AnyTest, ConversionTest) {
  {
    Any a;
    const Any ca;
    a = ca;
  }
  {
    Any a;
    a = Any();
  }
  {
    Any a;
    a = static_cast<const Any>((Any()));
  }
  {
    Any ai;
    ai = 1;
    Any ac;
    ac = 'c';
    Any as;
    as = std::string();
  }
  {
    Any ai;
    int i = 1;
    ai = i;
    Any ac;
    char c = 'c';
    ac = c;
    Any as;
    std::string s;
    as = s;
  }
  {
    Any a(1);
    ASSERT_THROW(AnyCast<char>(a), std::bad_cast);
  }
  {
    Any a(static_cast<int>(1));
    ASSERT_NO_THROW(AnyCast<int>(a));
  }
}

TEST(AnyTest, DataAvailTest) {
  {
    const char *data = "String Data";
    std::string s(data);
    Any a = s;
    ASSERT_STREQ(s.data(), data);
    ASSERT_STREQ(AnyCast<std::string>(a).data(), data);
  }
  {
    const char *data1 = "String Data1";
    const char *data2 = "String Data2";
    std::string s1(data1), s2(data2);
    Any a1 = s1, a2 = s2;
    ASSERT_STREQ(AnyCast<std::string>(a1).data(), data1);
    ASSERT_STREQ(AnyCast<std::string>(a2).data(), data2);
    a1.swap(a2);
    ASSERT_STREQ(AnyCast<std::string>(a1).data(), data2);
    ASSERT_STREQ(AnyCast<std::string>(a2).data(), data1);
  }
  {
    const char *data = "String Data";
    const char *empty_data = "";
    std::string s(data);
    Any a = s;
    std::string res;
    ASSERT_STREQ(res.data(), empty_data);
    ASSERT_STREQ(AnyCast<std::string>(a).data(), data);
    a.CopyTo(res);
    ASSERT_STREQ(res.data(), data);
  }
}

TEST(AnyTest, OtherTest) {
  {
    Any a;
    ASSERT_TRUE(a.empty());
  }
  {
    Any a(1);
    ASSERT_FALSE(a.empty());
  }
  {
    Any a(1);
    ASSERT_FALSE(a.empty());
    a.clear();
    ASSERT_TRUE(a.empty());
  }
  {
    Any a1(1), a2(2);
    ASSERT_EQ(a1.type_info(), a2.type_info());
  }
}

}  // namespace mytinyhttpd