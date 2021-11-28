#include <gtest/gtest.h>

#include "gtest/gtest-death-test.h"
#include "mytinyhttpd/base/async_logging.h"
#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/utils/constants.h"
#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

TEST(LogTest, LogStreamTest) {
  {
    LOG_TRACE << "a trace message";
    LOG_DEBUG << "a debug message";
    LOG_INFO << "a info message";
    LOG_WARN << "a warn message";
    LOG_ERROR << "a error message";
    ASSERT_DEATH(LOG_FATAL << "a fatal message", "");
    LOG_SYSERR << "a syserr message";
    ASSERT_DEATH(LOG_SYSFATAL << "a sysfatal message", "");
  }
  {
    int v = 0;
    char* p = nullptr;
    CHECK_NOTNULL(&v);
    ASSERT_DEATH(CHECK_NOTNULL(p), "");
  }
}

TEST(LogTest, AsyncLoggingTest) {}

}  // namespace mytinyhttpd

class LogEnvironment : public testing::Environment {
 public:
  virtual void SetUp() {
    using namespace mytinyhttpd;
    Logger::SetLogLevel(Logger::kTrace);
    TimeZone beijing(8 * 3600, "CST");
    Logger::SetTimeZone(beijing);
  }
};

int main(int argc, char* argv[]) {
  testing::AddGlobalTestEnvironment(new LogEnvironment);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}