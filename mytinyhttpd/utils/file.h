#ifndef MYTINYHTTPD_UTILS_FILE_H_
#define MYTINYHTTPD_UTILS_FILE_H_

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <zlib.h>

#include <fstream>
#include <stdexcept>
#include <string>

#include "mytinyhttpd/utils/noncopyable.h"
#include "mytinyhttpd/utils/slice.h"

namespace mytinyhttpd {

long GetFileSize(FILE* file);
long GetFileSize(std::ifstream& is);

class File : public noncopyable {
 public:
  File(const char* filename) : fp_(::fopen(filename, "rb")) {}

  ~File() {
    if (fp_) {
      ::fclose(fp_);
    }
  }

  bool IsValid() const { return fp_; }

  std::string ReadBytes(size_t n);

  uint8_t ReadUInt8();

  int32_t ReadInt32();

  int64_t ReadInt64();

 private:
  FILE* fp_;
};

class ReadSmallFile : public noncopyable {
 public:
  ReadSmallFile(Slice filename)
      : fd_(::open(filename.data(), O_RDONLY | O_CLOEXEC)), err_(0) {
    buf_[0] = '\0';
    if (fd_ < 0) {
      err_ = errno;
    }
  }
  ~ReadSmallFile() {
    if (fd_ >= 0) {
      ::close(fd_);
    }
  }

  int ReadToString(int max_size, std::string& content, int64_t* file_size,
                   int64_t* modify_time, int64_t* create_time);

  int ReadToBuffer(int* size);

  const char* buffer() const { return buf_; }

  static const int kBufferSize = 64 * 1024;

 private:
  int fd_;
  int err_;
  char buf_[kBufferSize];
};

inline int ReadFile(Slice filename, int max_size, std::string& content,
                    int64_t* file_size = nullptr,
                    int64_t* modify_time = nullptr,
                    int64_t* create_time = nullptr) {
  ReadSmallFile file(filename);
  return file.ReadToString(max_size, content, file_size, modify_time,
                           create_time);
}

class AppendFile : public noncopyable {
 public:
  explicit AppendFile(Slice filename)
      : fp_(::fopen(filename.data(), "ae" /* e -> O_CLOEXEC */)),
        written_bytes_(0) {
    assert(fp_);
    ::setbuffer(fp_, buffer_, sizeof(buffer_));
  }

  ~AppendFile() { ::fclose(fp_); }

  void Append(const char* logline, size_t len);

  void Flush() { ::fflush(fp_); }

  off_t written_bytes() const { return written_bytes_; }

  static const int kBufferSize = 64 * 1024;

 private:
  size_t Write(const char* logline, size_t len) {
    // faster but not thread-safe
    return ::fwrite_unlocked(logline, 1, len, fp_);
  }

  FILE* fp_;
  char buffer_[kBufferSize];
  off_t written_bytes_;
};

class GzipFile : public noncopyable {
 public:
  GzipFile(GzipFile&& x) noexcept : file_(x.file_) { x.file_ = nullptr; }

  ~GzipFile() {
    if (file_) {
      ::gzclose(file_);
    }
  }

  GzipFile& operator=(GzipFile&& x) noexcept {
    swap(x);
    return *this;
  }

  bool IsValid() const { return file_ != nullptr; }
  void swap(GzipFile& x) { std::swap(file_, x.file_); }

#if ZLIB_VERNUM >= 0x1240
  bool SetBuffer(int size) { return ::gzbuffer(file_, size) == 0; }
#endif

  // return the number of uncompressed bytes actually read, 0 for eof, -1 for
  // error
  int Read(void* buf, int len) { return ::gzread(file_, buf, len); }

  // return the number of uncompressed bytes actually written
  int Write(Slice buf) { return ::gzwrite(file_, buf.data(), buf.size()); }

  // number of uncompressed bytes
  off_t Tell() const { return ::gztell(file_); }

#if ZLIB_VERNUM >= 0x1240
  // number of compressed bytes
  off_t Offset() const { return ::gzoffset(file_); }
#endif

  int Flush(int f) { return ::gzflush(file_, f); }

  static GzipFile OpenForRead(Slice filename) {
    return GzipFile(::gzopen(filename.data(), "rbe"));
  }

  static GzipFile OpenForAppend(Slice filename) {
    return GzipFile(::gzopen(filename.data(), "abe"));
  }

  static GzipFile OpenForWriteExclusive(Slice filename) {
    return GzipFile(::gzopen(filename.data(), "wbxe"));
  }

  static GzipFile OpenForWriteTruncate(Slice filename) {
    return GzipFile(::gzopen(filename.data(), "wbe"));
  }

 private:
  explicit GzipFile(gzFile file) : file_(file) {}

  gzFile file_;
};

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_UTILS_FILE_H_