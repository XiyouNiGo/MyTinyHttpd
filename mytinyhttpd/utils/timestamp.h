#ifndef MYTINYHTTPD_UTILS_TIMESTAMP_H_
#define MYTINYHTTPD_UTILS_TIMESTAMP_H_

#include <boost/operators.hpp>
#include <cstdint>
#include <string>

#include "mytinyhttpd/utils/copyable.h"

namespace mytinyhttpd {

class Timestamp : public mytinyhttpd::copyable,
                  public boost::equality_comparable<Timestamp>,
                  public boost::less_than_comparable<Timestamp>,
                  public boost::subtractable<Timestamp>,
                  public boost::addable<Timestamp> {
 public:
  Timestamp() : micro_seconds_since_epoch_(0) {}

  // int64_t must stand for micro second
  explicit Timestamp(int64_t micro_seconds_since_epoch)
      : micro_seconds_since_epoch_(micro_seconds_since_epoch) {}

  explicit operator int64_t() /* micro seconds */ const {
    return micro_seconds_since_epoch_;
  }

  // double must stand for second
  explicit Timestamp(double seconds_since_epoch)
      : micro_seconds_since_epoch_(static_cast<int64_t>(
            seconds_since_epoch * Timestamp::kMicroSecondsPerSecond)) {}

  explicit operator double() /* seconds */ const {
    return static_cast<double>(micro_seconds_since_epoch_) /
           Timestamp::kMicroSecondsPerSecond;
  }

  void swap(Timestamp& timestamp) {
    std::swap(micro_seconds_since_epoch_, timestamp.micro_seconds_since_epoch_);
  }

  std::string ToString() const;
  std::string ToFormattedString(bool show_microseconds = true) const;

  bool Valid() const { return micro_seconds_since_epoch_ > 0; }

  int64_t micro_seconds_since_epoch() const {
    return micro_seconds_since_epoch_;
  }
  time_t seconds_since_epoch() const {
    return static_cast<time_t>(micro_seconds_since_epoch_ /
                               kMicroSecondsPerSecond);
  }

  Timestamp& operator+=(Timestamp timestamp) {
    micro_seconds_since_epoch_ += timestamp.micro_seconds_since_epoch_;
    return *this;
  }

  Timestamp& operator-=(Timestamp timestamp) {
    micro_seconds_since_epoch_ -= timestamp.micro_seconds_since_epoch_;
    return *this;
  }

  static Timestamp Now();
  static Timestamp Invalid() { return Timestamp(); }

  static Timestamp FromUnixTime(time_t t) { return FromUnixTime(t, 0); }

  static Timestamp FromUnixTime(time_t t, int microseconds) {
    return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond +
                     microseconds);
  }

  static const int kMicroSecondsPerSecond = 1000 * 1000;
  static const int kNanoSecondsPerMicroSecond = 1000;
  static const int64_t kNanoSecondsPerSecond = 1000 * 1000 * 1000;

 private:
  int64_t micro_seconds_since_epoch_;

  friend inline bool operator<(Timestamp lhs, Timestamp rhs);
  friend inline bool operator==(Timestamp lhs, Timestamp rhs);
};

inline bool operator<(Timestamp lhs, Timestamp rhs) {
  return lhs.micro_seconds_since_epoch() < rhs.micro_seconds_since_epoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
  return lhs.micro_seconds_since_epoch() == rhs.micro_seconds_since_epoch();
}

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_UTILS_TIMESTAMP_H_