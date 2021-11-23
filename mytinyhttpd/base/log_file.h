#ifndef MYTINYHTTPD_BASE_LOG_FILE_H_
#define MYTINYHTTPD_BASE_LOG_FILE_H_

#include <memory>

#include "mytinyhttpd/base/mutex.h"
#include "mytinyhttpd/utils/file.h"

namespace mytinyhttpd {

class LogFile : public noncopyable {
 public:
  LogFile(const std::string& basename, off_t roll_size,
          bool is_thread_safe = true, int flush_interval = 3,
          int check_every_n = 1024);
  ~LogFile() = default;

  void Append(const char* logline, int len);
  void Flush();
  bool RollFile();

 private:
  void AppendUnlocked(const char* logline, int len);

  static std::string GetLogFilename(const std::string& basename, time_t* now);

  const std::string basename_;
  const off_t roll_size_;
  const int flush_interval_;
  const int check_every_n_;

  int count_;

  std::unique_ptr<MutexLock> mutex_;
  time_t start_of_period_;
  time_t last_roll_;
  time_t last_flush_;
  std::unique_ptr<AppendFile> file_;

  // RollFile every day
  const static int kRollPerSeconds_ = 60 * 60 * 24;
};

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_BASE_LOG_FILE_H_
