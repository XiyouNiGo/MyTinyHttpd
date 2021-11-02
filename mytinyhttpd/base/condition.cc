#include "mytinyhttpd/base/condition.h"

#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

bool Condition::WaitForSeconds(double seconds) {
  struct timespec walltime;
  // CLOCK_MONOTONIC -> jiffies time
  // CLOCK_REALTIME -> wall time
  clock_gettime(CLOCK_REALTIME, &walltime);

  int64_t nanoseconds = static_cast<int64_t>(seconds * Timestamp::kNanoSecondsPerSecond);
  walltime.tv_sec += static_cast<time_t>((walltime.tv_nsec + nanoseconds) /
                                        Timestamp::kNanoSecondsPerSecond);
  walltime.tv_nsec = static_cast<long>((walltime.tv_nsec + nanoseconds) %
                                      Timestamp::kNanoSecondsPerSecond);

  MutexLock::UnassignGuard guard(mutex_);
  return ETIMEDOUT ==
         pthread_cond_timedwait(&cond_, mutex_.GetMutex(), &walltime);
}

}  // namespace mytinyhttpd