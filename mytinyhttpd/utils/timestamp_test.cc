#include "mytinyhttpd/utils/timestamp.h"

#include <gtest/gtest.h>

namespace mytinyhttpd {

TEST(TimestampTest, TypeConvertionTest) {
  {
    int64_t i = 10;
    Timestamp timestamp(i);
    ASSERT_EQ(static_cast<int64_t>(timestamp), i);
  }
  {
    double d = 10;
    Timestamp timestamp(d);
    ASSERT_DOUBLE_EQ(static_cast<double>(timestamp), d);
  }
}

TEST(TimestampTest, ToStringTest) {
  Timestamp now = Timestamp::Now();
  std::cout << now.ToString() << std::endl;
  std::cout << now.ToFormattedString() << std::endl;
  std::cout << now.ToFormattedString(false) << std::endl;
}

TEST(TimestampTest, StaticMethodTest) {
  ASSERT_EQ(Timestamp::Invalid().GetMicroSecondsSinceEpoch(), 0);
  ASSERT_NE(Timestamp::Now().GetMicroSecondsSinceEpoch(), 0);
}

TEST(TimestampTest, ArithmeticTest) {
  {
    Timestamp timestamp1(static_cast<int64_t>(1));
    Timestamp timestamp2(static_cast<int64_t>(2));
    ASSERT_EQ((timestamp1 + timestamp2).GetMicroSecondsSinceEpoch(), 3);
  }
  {
    Timestamp timestamp1(static_cast<int64_t>(3));
    Timestamp timestamp2(static_cast<int64_t>(2));
    ASSERT_EQ((timestamp1 - timestamp2).GetMicroSecondsSinceEpoch(), 1);
  }
  {
    Timestamp timestamp1(static_cast<int64_t>(1));
    Timestamp timestamp2(static_cast<int64_t>(2));
    Timestamp timestamp3(static_cast<int64_t>(3));
    ASSERT_EQ(timestamp1 + timestamp2, timestamp3);
  }
  {
    Timestamp timestamp1(static_cast<int64_t>(1));
    Timestamp timestamp2(static_cast<int64_t>(2));
    Timestamp timestamp3(static_cast<int64_t>(4));
    ASSERT_NE(timestamp1 + timestamp2, timestamp3);
  }
  {
    Timestamp timestamp1(static_cast<int64_t>(1));
    Timestamp timestamp2(static_cast<int64_t>(2));
    ASSERT_TRUE(timestamp1 < timestamp2);
  }
  {
    Timestamp timestamp1(static_cast<int64_t>(1));
    Timestamp timestamp2(static_cast<int64_t>(2));
    ASSERT_TRUE(timestamp2 > timestamp1);
  }
}

TEST(TimestampTest, OtherMethodTest) {
  { ASSERT_EQ(Timestamp::Invalid().Valid(), false); }
}

}  // namespace mytinyhttpd