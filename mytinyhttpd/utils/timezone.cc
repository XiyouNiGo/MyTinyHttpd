#include "mytinyhttpd/utils/timezone.h"

#include <assert.h>
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <strings.h>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include "mytinyhttpd/utils/date.h"
#include "mytinyhttpd/utils/file.h"
#include "mytinyhttpd/utils/noncopyable.h"

namespace mytinyhttpd {

namespace detail {

struct Transition {
  time_t gmttime;
  time_t localtime;
  int localtime_idx;

  Transition(time_t t, time_t l, int local_idx)
      : gmttime(t), localtime(l), localtime_idx(local_idx) {}
};

struct Comp {
  bool compare_gmt;

  Comp(bool gmt) : compare_gmt(gmt) {}

  bool operator()(const Transition& lhs, const Transition& rhs) const {
    if (compare_gmt)
      return lhs.gmttime < rhs.gmttime;
    else
      return lhs.localtime < rhs.localtime;
  }

  bool equal(const Transition& lhs, const Transition& rhs) const {
    if (compare_gmt)
      return lhs.gmttime == rhs.gmttime;
    else
      return lhs.localtime == rhs.localtime;
  }
};

struct Localtime {
  time_t gmt_offset;
  bool is_dst;
  int arrb_idx;

  Localtime(time_t offset, bool dst, int arrb)
      : gmt_offset(offset), is_dst(dst), arrb_idx(arrb) {}
};

inline void FillHms(unsigned seconds, struct tm* utc) {
  utc->tm_sec = static_cast<int>(seconds % 60);
  unsigned minutes = seconds / 60;
  utc->tm_min = static_cast<int>(minutes % 60);
  utc->tm_hour = static_cast<int>(minutes / 60);
}

}  // namespace detail

struct TimeZone::Rep {
  std::vector<detail::Transition> transitions;
  std::vector<detail::Localtime> localtimes;
  std::vector<std::string> names;
  std::string abbreviation;
};

bool ReadTimeZoneFile(const char* zonefile, struct TimeZone::Rep* data) {
  File f(zonefile);
  if (f.Valid()) {
    try {
      std::string head = f.ReadBytes(4);
      if (head != "TZif") {
        throw std::logic_error("bad head");
      }
      std::string version = f.ReadBytes(1);
      f.ReadBytes(15);

      int32_t isgmtcnt = f.ReadInt32();
      int32_t isstdcnt = f.ReadInt32();
      // int32_t leapcnt = f.ReadInt32();
      int32_t timecnt = f.ReadInt32();
      int32_t typecnt = f.ReadInt32();
      int32_t charcnt = f.ReadInt32();

      std::vector<int32_t> trans;
      std::vector<int> localtimes;
      trans.reserve(timecnt);
      for (int i = 0; i < timecnt; ++i) {
        trans.push_back(f.ReadInt32());
      }

      for (int i = 0; i < timecnt; ++i) {
        uint8_t local = f.ReadUInt8();
        localtimes.push_back(local);
      }

      for (int i = 0; i < typecnt; ++i) {
        int32_t gmtoff = f.ReadInt32();
        uint8_t isdst = f.ReadUInt8();
        uint8_t abbrind = f.ReadUInt8();

        data->localtimes.push_back(detail::Localtime(gmtoff, isdst, abbrind));
      }

      for (int i = 0; i < timecnt; ++i) {
        int local_idx = localtimes[i];
        time_t localtime = trans[i] + data->localtimes[local_idx].gmt_offset;
        data->transitions.push_back(
            detail::Transition(trans[i], localtime, local_idx));
      }

      data->abbreviation = f.ReadBytes(charcnt);

      // for (int i = 0; i < leapcnt; ++i) {
      //   int32_t leaptime = f.ReadInt32();
      //   int32_t cumleap = f.ReadInt32();
      // }
      (void)isstdcnt;
      (void)isgmtcnt;
    } catch (std::logic_error& e) {
      fprintf(stderr, "%s\n", e.what());
    }
  }
  return true;
}

const detail::Localtime* FindLocaltime(const TimeZone::Rep& rep,
                                       detail::Transition sentry,
                                       detail::Comp comp) {
  const detail::Localtime* local = NULL;

  if (rep.transitions.empty() || comp(sentry, rep.transitions.front())) {
    local = &rep.localtimes.front();
  } else {
    std::vector<detail::Transition>::const_iterator trans_i = lower_bound(
        rep.transitions.begin(), rep.transitions.end(), sentry, comp);
    if (trans_i != rep.transitions.end()) {
      if (!comp.equal(sentry, *trans_i)) {
        assert(trans_i != data.transitions.begin());
        --trans_i;
      }
      local = &rep.localtimes[trans_i->localtime_idx];
    } else {
      local = &rep.localtimes[rep.transitions.back().localtime_idx];
    }
  }

  return local;
}

TimeZone::TimeZone(const char* zonefile) : rep_(new TimeZone::Rep) {
  if (!ReadTimeZoneFile(zonefile, rep_.get())) {
    rep_.reset();
  }
}

TimeZone::TimeZone(int east_of_utc, const char* name)
    : rep_(new TimeZone::Rep) {
  rep_->localtimes.push_back(detail::Localtime(east_of_utc, false, 0));
  rep_->abbreviation = name;
}

struct tm TimeZone::ToLocalTime(time_t seconds) const {
  struct tm localTime;
  bzero(&localTime, sizeof(localTime));
  assert(data_ != NULL);
  const Rep& data(*rep_);

  detail::Transition sentry(seconds, 0, 0);
  const detail::Localtime* local =
      FindLocaltime(data, sentry, detail::Comp(true));

  if (local) {
    time_t localSeconds = seconds + local->gmt_offset;
    ::gmtime_r(&localSeconds, &localTime);
    localTime.tm_isdst = local->is_dst;
    localTime.tm_gmtoff = local->gmt_offset;
    localTime.tm_zone = &data.abbreviation[local->arrb_idx];
  }

  return localTime;
}

time_t TimeZone::FromLocalTime(const struct tm& localTm) const {
  assert(rep_ != NULL);
  const Rep& data(*rep_);

  struct tm tmp = localTm;
  time_t seconds = ::timegm(&tmp);
  detail::Transition sentry(0, seconds, 0);
  const detail::Localtime* local =
      FindLocaltime(data, sentry, detail::Comp(false));
  if (localTm.tm_isdst) {
    struct tm try_tm = ToLocalTime(seconds - local->gmt_offset);
    if (!try_tm.tm_isdst && try_tm.tm_hour == localTm.tm_hour &&
        try_tm.tm_min == localTm.tm_min) {
      seconds -= 3600;
    }
  }
  return seconds - local->gmt_offset;
}

const int kSecondsPerDay = 24 * 60 * 60;

struct tm TimeZone::ToUtcTime(time_t secondsSinceEpoch, bool yday) {
  struct tm utc;
  bzero(&utc, sizeof(utc));
  utc.tm_zone = "GMT";
  int seconds = static_cast<int>(secondsSinceEpoch % kSecondsPerDay);
  int days = static_cast<int>(secondsSinceEpoch / kSecondsPerDay);
  if (seconds < 0) {
    seconds += kSecondsPerDay;
    --days;
  }
  detail::FillHms(seconds, &utc);
  Date date(days + Date::kJulianDayOf1970_01_01);
  Date::YearMonthDay ymd = date.ToYearMonthDay();
  utc.tm_year = ymd.year - 1900;
  utc.tm_mon = ymd.month - 1;
  utc.tm_mday = ymd.day;
  utc.tm_wday = date.ToWeekDay();

  if (yday) {
    Date startOfYear(ymd.year, 1, 1);
    utc.tm_yday = date.julian_day_number() - startOfYear.julian_day_number();
  }
  return utc;
}

time_t TimeZone::FromUtcTime(const struct tm& utc) {
  return FromUtcTime(utc.tm_year + 1900, utc.tm_mon + 1, utc.tm_mday,
                     utc.tm_hour, utc.tm_min, utc.tm_sec);
}

time_t TimeZone::FromUtcTime(int year, int month, int day, int hour, int minute,
                             int seconds) {
  Date date(year, month, day);
  int secondsInDay = hour * 3600 + minute * 60 + seconds;
  time_t days = date.julian_day_number() - Date::kJulianDayOf1970_01_01;
  return days * kSecondsPerDay + secondsInDay;
}

}  // namespace mytinyhttpd