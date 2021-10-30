#include "gtest/gtest.h"

namespace mytinyhttpd {

TEST(Gtest, Hello) {
  ASSERT_TRUE(false) << "Hello Gtest" << std::endl;
}

}// mytinyhttpd

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}