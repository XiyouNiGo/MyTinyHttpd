#include "mytinyhttpd/base/logging.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sstream>

#include "mytinyhttpd/base/current_thread.h"
#include "mytinyhttpd/utils/timestamp.h"
#include "mytinyhttpd/utils/timezone.h"

namespace mytinyhttpd {

__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_last_second;

const char* strerror_tl(int old_errno) {
  return strerror_r(old_errno, t_errnobuf, sizeof t_errnobuf);
}

Logger::LogLevel InitLogLevel() {
  if (::getenv("MYTINYHTTPD_LOG_TRACE"))
    return Logger::kTrace;
  else if (::getenv("MYTINYHTTPD_LOG_DEBUG"))
    return Logger::kDebug;
  else
    return Logger::kInfo;
}

Logger::LogLevel g_log_level = InitLogLevel();

const char* LogLevelName[Logger::kNumLogLevels] = {
    "kTrace ", "kDebug ", "kInfo  ", "kWarn  ", "kError ", "kFatal ",
};

class T {
 public:
  T(const char* str, unsigned len) : str_(str), len_(len) {
    assert(strlen(str) == len_);
  }

  const char* str_;
  const unsigned len_;
};

inline LogStream& operator<<(LogStream& s, T v) {
  s.Append(v.str_, v.len_);
  return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v) {
  s.Append(v.data_, v.size_);
  return s;
}

void DefaultOutputFunc(const char* msg, int len) {
  ::fwrite(msg, 1, len, stdout);
}

void DefaultFlushFunc() { ::fflush(stdout); }

Logger::OutputFunc g_output_func = DefaultOutputFunc;
Logger::FlushFunc g_flush_func = DefaultFlushFunc;
TimeZone g_log_time_zone;

Logger::Impl::Impl(LogLevel level, int old_errno, const SourceFile& file,
                   int line)
    : time_(Timestamp::Now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(file) {
  FormatTime();
  CurrentThread::tid();
  stream_ << T(CurrentThread::tid_string(), CurrentThread::tid_string_length());
  stream_ << T(LogLevelName[level], 7);
  if (old_errno != 0) {
    stream_ << strerror_tl(old_errno) << " (errno=" << old_errno << ") ";
  }
}

void Logger::Impl::FormatTime() {
  int64_t micro_seconds_since_epoch = time_.micro_seconds_since_epoch();
  time_t seconds = static_cast<time_t>(micro_seconds_since_epoch /
                                       Timestamp::kMicroSecondsPerSecond);
  int microseconds = static_cast<int>(micro_seconds_since_epoch %
                                      Timestamp::kMicroSecondsPerSecond);
  // only format prefix every second
  if (seconds != t_last_second) {
    t_last_second = seconds;
    struct tm tm_time;
    if (g_log_time_zone.IsValid()) {
      tm_time = g_log_time_zone.ToLocalTime(seconds);
    } else {
      ::gmtime_r(&seconds, &tm_time);
    }

    int len =
        snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    assert(len == 17);
    (void)len;
  }

  if (g_log_time_zone.IsValid()) {
    Fmt us(".%06d ", microseconds);
    assert(us.length() == 8);
    stream_ << T(t_time, 17) << T(us.data(), 8);
  } else {
    Fmt us(".%06dZ ", microseconds);
    assert(us.length() == 9);
    stream_ << T(t_time, 17) << T(us.data(), 9);
  }
}

void Logger::Impl::Finish() {
  stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::Logger(SourceFile file, int line) : impl_(kInfo, 0, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
    : impl_(level, 0, file, line) {
  impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
    : impl_(level, 0, file, line) {}

Logger::Logger(SourceFile file, int line, bool to_abort)
    : impl_(to_abort ? kFatal : kError, errno, file, line) {}

Logger::~Logger() {
  impl_.Finish();
  const LogStream::Buffer& buf(stream().buffer());
  g_output_func(buf.data(), buf.length());
  if (impl_.level_ == kFatal) {
    g_flush_func();
    abort();
  }
}

void Logger::SetLogLevel(Logger::LogLevel level) { g_log_level = level; }

void Logger::SetOutput(OutputFunc out) { g_output_func = out; }

void Logger::SetFlush(FlushFunc flush) { g_flush_func = flush; }

void Logger::SetTimeZone(const TimeZone& tz) { g_log_time_zone = tz; }

}  // namespace mytinyhttpd
