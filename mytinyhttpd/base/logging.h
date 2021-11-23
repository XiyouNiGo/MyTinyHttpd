#ifndef MYTINYHTTPD_BASE_LOGGING_H_
#define MYTINYHTTPD_BASE_LOGGING_H_

#include <functional>

#include "mytinyhttpd/base/log_stream.h"
#include "mytinyhttpd/utils/timestamp.h"
#include "mytinyhttpd/utils/timezone.h"

namespace mytinyhttpd {

class Logger {
 public:
  enum LogLevel : char {
    kTrace,
    kDebug,
    kInfo,
    kWarn,
    kError,
    kFatal,
    kNumLogLevels,
  };

  // compile time calculation of basename of source file
  class SourceFile {
   public:
    template <int N>
    SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1) {
      const char* slash =
          strrchr(data_, '/');  // will call __builtin_strrchr in release mode
      if (slash) {
        data_ = slash + 1;
        size_ -= static_cast<int>(data_ - arr);
      }
    }

    explicit SourceFile(const char* filename) : data_(filename) {
      const char* slash = strrchr(filename, '/');
      if (slash) {
        data_ = slash + 1;
      }
      size_ = static_cast<int>(strlen(data_));
    }

    const char* data_;
    int size_;
  };

  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char* func);
  Logger(SourceFile file, int line, bool to_abort);
  ~Logger();

  LogStream& stream() { return impl_.stream_; }

  static LogLevel log_level();
  static void SetLogLevel(LogLevel level);

  typedef std::function<void(const char*, int)> OutputFunc;
  typedef std::function<void()> FlushFunc;
  static void SetOutput(OutputFunc);
  static void SetFlush(FlushFunc);
  static void SetTimeZone(const TimeZone& tz);

 private:
  class Impl {
   public:
    Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
    void FormatTime();
    void Finish();

    Timestamp time_;
    LogStream stream_;
    LogLevel level_;
    int line_;
    SourceFile basename_;
  };

  // not a pointer
  Impl impl_;
};

extern Logger::LogLevel g_log_level;

inline Logger::LogLevel Logger::log_level() { return g_log_level; }

#define LOG_TRACE                           \
  if (Logger::log_level() <= Logger::kTrace) \
  Logger(__FILE__, __LINE__, Logger::kTrace, __func__).stream()
#define LOG_DEBUG                           \
  if (Logger::log_level() <= Logger::kDebug) \
  Logger(__FILE__, __LINE__, Logger::kDebug, __func__).stream()
#define LOG_INFO \
  if (Logger::log_level() <= Logger::kInfo) Logger(__FILE__, __LINE__).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::kWarn).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::kError).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::kFatal).stream()
#define LOG_SYSERR Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int old_errno);

#define CHECK_NOTNULL(val) \
  CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non nullptr", (val))

template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char* names, T* ptr) {
  if (ptr == nullptr) {
    Logger(file, line, Logger::kFatal).stream() << names;
  }
  return ptr;
}

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_LOGGING_H_
