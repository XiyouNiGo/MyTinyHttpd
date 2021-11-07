#ifndef MYTINYHTTPD_UTILS_DATE_H_
#define MYTINYHTTPD_UTILS_DATE_H_

#include <boost/operators.hpp>
#include <string>

#include "mytinyhttpd/utils/copyable.h"

namespace mytinyhttpd {

// convert year month day and julianDay
class Date : public mytinyhttpd::copyable,
             public boost::less_than_comparable<Date>,
             public boost::equality_comparable<Date> {
 public:
  struct YearMonthDay {
    int year;
    int month;
    int day;
  };

  static const int kDaysPerWeek = 7;
  static const int kJulianDayOf1970_01_01;

  Date() : julian_day_number_(0) {}

  Date(int year, int month, int day);

  explicit Date(int julian_day_num) : julian_day_number_(julian_day_num) {}

  explicit Date(const struct tm&);

  void swap(Date& date) {
    std::swap(julian_day_number_, date.julian_day_number_);
  }

  bool Valid() const { return julian_day_number_ > 0; }

  std::string ToIsoString() const;

  struct YearMonthDay ToYearMonthDay() const;

  int year() const { return ToYearMonthDay().year; }

  int month() const { return ToYearMonthDay().month; }

  int day() const { return ToYearMonthDay().day; }

  int ToWeekDay() const { return (julian_day_number_ + 1) % kDaysPerWeek; }

  int julian_day_number() const { return julian_day_number_; }

 private:
  int julian_day_number_;
};

inline bool operator<(Date x, Date y) {
  return x.julian_day_number() < y.julian_day_number();
}

inline bool operator==(Date x, Date y) {
  return x.julian_day_number() == y.julian_day_number();
}

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_UTILS_DATE_H_