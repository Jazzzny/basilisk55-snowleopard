/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef mozilla_pkix_Time_h
#define mozilla_pkix_Time_h

#include <stdint.h>
#include <ctime>
#include <limits>

#include "pkix/Result.h"

namespace mozilla {
namespace pkix {

// Time with a range from the first second of year 0 (AD) through at least the
// last second of year 9999, which is the range of legal times in X.509 and
// OCSP. This type has second-level precision. The time zone is always UTC.
//
// Pass by value, not by reference.
class Time final {
 public:
  // Construct an uninitialized instance.
  //
  // This will fail to compile because there is no default constructor:
  //    Time x;
  //
  // This will succeed, leaving the time uninitialized:
  //    Time x(Time::uninitialized);
  enum Uninitialized { uninitialized };
  explicit Time(Uninitialized) {}

  bool operator==(const Time& other) const {
    return elapsedSecondsAD == other.elapsedSecondsAD;
  }
  bool operator>(const Time& other) const {
    return elapsedSecondsAD > other.elapsedSecondsAD;
  }
  bool operator>=(const Time& other) const {
    return elapsedSecondsAD >= other.elapsedSecondsAD;
  }
  bool operator<(const Time& other) const {
    return elapsedSecondsAD < other.elapsedSecondsAD;
  }
  bool operator<=(const Time& other) const {
    return elapsedSecondsAD <= other.elapsedSecondsAD;
  }

  Result AddSeconds(uint64_t seconds) {
    if (std::numeric_limits<uint64_t>::max() - elapsedSecondsAD < seconds) {
      return Result::FATAL_ERROR_INVALID_ARGS;  // integer overflow
    }
    elapsedSecondsAD += seconds;
    return Success;
  }

  Result SubtractSeconds(uint64_t seconds) {
    if (seconds > elapsedSecondsAD) {
      return Result::FATAL_ERROR_INVALID_ARGS;  // integer overflow
    }
    elapsedSecondsAD -= seconds;
    return Success;
  }

  static const uint64_t ONE_DAY_IN_SECONDS =
      UINT64_C(24) * UINT64_C(60) * UINT64_C(60);

 private:
  // This constructor is hidden to prevent accidents like this:
  //
  //    Time foo(time_t t)
  //    {
  //      // WRONG! 1970-01-01-00:00:00 == time_t(0), but not Time(0)!
  //      return Time(t);
  //    }
  explicit Time(uint64_t aElapsedSecondsAD)
      : elapsedSecondsAD(aElapsedSecondsAD) {}
  friend Time TimeFromElapsedSecondsAD(uint64_t);
  friend class Duration;

  uint64_t elapsedSecondsAD;
};

inline Time TimeFromElapsedSecondsAD(uint64_t aElapsedSecondsAD) {
  return Time(aElapsedSecondsAD);
}

Time Now();

// Note the epoch is the unix epoch (ie 00:00:00 UTC, 1 January 1970)
Time TimeFromEpochInSeconds(uint64_t secondsSinceEpoch);

class Duration final {
 public:
  Duration(Time timeA, Time timeB)
      : durationInSeconds(
            timeA < timeB ? timeB.elapsedSecondsAD - timeA.elapsedSecondsAD
                          : timeA.elapsedSecondsAD - timeB.elapsedSecondsAD) {}

  explicit Duration(uint64_t aDurationInSeconds)
      : durationInSeconds(aDurationInSeconds) {}

  bool operator>(const Duration& other) const {
    return durationInSeconds > other.durationInSeconds;
  }
  bool operator<(const Duration& other) const {
    return durationInSeconds < other.durationInSeconds;
  }

 private:
  uint64_t durationInSeconds;
};
}
}  // namespace mozilla::pkix

#endif  // mozilla_pkix_Time_h
