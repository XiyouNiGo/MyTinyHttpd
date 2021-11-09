#include "mytinyhttpd/utils/file.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>

#include "mytinyhttpd/utils/slice.h"

namespace mytinyhttpd {

std::string File::ReadBytes(size_t n) {
  char buf[n];
  size_t nr = fread(buf, 1, n, fp_);
  if (nr != n) throw std::logic_error("no enough data");
  return std::string(buf, n);
}

uint8_t File::ReadUInt8() {
  uint8_t x = 0;
  size_t nr = fread(&x, 1, sizeof(uint8_t), fp_);
  if (nr != sizeof(uint8_t)) throw std::logic_error("bad uint8_t data");
  return x;
}

int32_t File::ReadInt32() {
  int32_t x = 0;
  size_t nr = fread(&x, 1, sizeof(int32_t), fp_);
  if (nr != sizeof(int32_t)) throw std::logic_error("bad int32_t data");
  return be32toh(x);
}

int64_t File::ReadInt64() {
  int64_t x = 0;
  size_t nr = fread(&x, 1, sizeof(int64_t), fp_);
  if (nr != sizeof(int64_t)) throw std::logic_error("bad int64_t data");
  return be64toh(x);
}

int ReadSmallFile::ReadToString(int max_size, std::string& content,
                                int64_t* file_size, int64_t* modify_time,
                                int64_t* create_time) {
  static_assert(sizeof(off_t) == 8, "_FILE_OFFSET_BITS = 64");
  assert(content != NULL);
  int err = err_;
  if (fd_ >= 0) {
    content.clear();
    if (file_size) {
      struct stat statbuf;
      if (fstat(fd_, &statbuf) == 0) {
        if (S_ISREG(statbuf.st_mode)) {
          *file_size = statbuf.st_size;
          // pre-allocation
          content.reserve(static_cast<int>(
              std::min(static_cast<int64_t>(max_size), *file_size)));
        } else if (S_ISDIR(statbuf.st_mode)) {
          err = EISDIR;
        }
        if (modify_time) {
          *modify_time = statbuf.st_mtime;
        }
        if (create_time) {
          *create_time = statbuf.st_ctime;
        }
      } else {
        err = errno;
      }
    }

    while (content.size() < static_cast<size_t>(max_size)) {
      size_t to_read = std::min(static_cast<size_t>(max_size) - content.size(),
                                sizeof(buf_));
      ssize_t n = read(fd_, buf_, to_read);
      if (n > 0) {
        content.append(buf_, n);
      } else {
        if (n < 0) {
          err = errno;
        }
        break;
      }
    }
  }
  return err;
}

int ReadSmallFile::ReadToBuffer(int* size) {
  int err = err_;
  if (fd_ >= 0) {
    ssize_t n = read(fd_, buf_, sizeof(buf_) - 1);
    if (n >= 0) {
      if (size) {
        *size = static_cast<int>(n);
      }
      buf_[n] = '\0';
    } else {
      err = errno;
    }
  }
  return err;
}

void AppendFile::Append(const char* logline, const size_t len) {
  size_t written = 0;

  while (written != len) {
    size_t remain = len - written;
    size_t n = Write(logline + written, remain);
    if (n != remain) {
      int err = ferror(fp_);
      if (err) {
        fprintf(stderr, "AppendFile::append() failed %s\n", strerror(err));
        break;
      }
    }
    written += n;
  }

  written_bytes_ += static_cast<off_t>(written);
}

}  // namespace mytinyhttpd