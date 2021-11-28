#include "mytinyhttpd/utils/constants.h"

#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>

namespace mytinyhttpd {

TEST(ConstantsTest, CheckRetValTest) {
  CheckRetVal(0, 0, "CheckRetValTest");
  ASSERT_DEATH(CheckRetVal(1, 0, "CheckRetValTest"), "CheckRetValTest");
}

}  // namespace mytinyhttpd