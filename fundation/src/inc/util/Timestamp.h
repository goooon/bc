#ifndef GUARD_TimeStamp_h__
#define GUARD_TimeStamp_h__

#include "../core/Types.h"
#include <time.h>

/// A Timestamp stores a monotonic* time value
/// with (theoretical) microseconds resolution.
/// Timestamps can be compared with each other
/// and simple arithmetics are supported.
///
/// [*] Note that Timestamp values are only monotonic as
/// long as the systems's clock is monotonic as well
/// (and not, e.g. set back).
///
/// Timestamps are UTC (Coordinated Universal Time)
/// based and thus independent of the timezone
/// in effect on the system.
class Timestamp
{
	friend class DateTime;
public:
	typedef s64 TimeVal;    /// monotonic UTC time value in microsecond resolution
	typedef s64 UtcTimeVal; /// monotonic UTC time value in 100 nanosecond resolution
	typedef s64 TimeDiff;   /// difference between two timestamps in microseconds
public:
	Timestamp() { update(); };
	/// Creates a timestamp with the current time.

	Timestamp(TimeVal tv) :ts(tv) {};
	/// Creates a timestamp from the given time value.

	Timestamp(const Timestamp& other) :ts(other.ts) {};
	/// Copy constructor.

	~Timestamp() {};
	/// Destroys the timestamp

	void update();
	/// Updates the Timestamp with the current time.

	void update(TimeVal milliseconds);

	void swap(Timestamp& timestamp) { TimeVal t = ts; ts = timestamp.ts; timestamp.ts = t; };
	/// Swaps the Timestamp with another one.

	TimeVal getValue() { return ts; }

	bool operator == (const Timestamp& ts) const { return ts == ts.ts; }
	bool operator != (const Timestamp& ts) const { return ts != ts.ts; }
	bool operator >  (const Timestamp& ts) const { return ts > ts.ts; };
	bool operator >= (const Timestamp& ts) const { return ts >= ts.ts; };
	bool operator <  (const Timestamp& ts) const { return ts < ts.ts; };
	bool operator <= (const Timestamp& ts) const { return ts <= ts.ts; };
private:
	TimeVal ts;
};

class DateTime
{
public:
	enum Months
	{
		JANUARY = 1,
		FEBRUARY,
		MARCH,
		APRIL,
		MAY,
		JUNE,
		JULY,
		AUGUST,
		SEPTEMBER,
		OCTOBER,
		NOVEMBER,
		DECEMBER
	};

	enum DaysOfWeek
	{
		SUNDAY = 0,
		MONDAY,
		TUESDAY,
		WEDNESDAY,
		THURSDAY,
		FRIDAY,
		SATURDAY
	};

	DateTime() {
		struct tm *tblock;
		Timestamp now;
		tblock = localtime(&now.ts);
		tm_sec = tblock->tm_sec;
		tm_min = tblock->tm_min;
		tm_hour = tblock->tm_hour;
		tm_mday = tblock->tm_mday;
		tm_mon = tblock->tm_mon;
		tm_year = tblock->tm_year;
		tm_wday = tblock->tm_wday;
		tm_yday = tblock->tm_yday;
		tm_isdst = tblock->tm_isdst;
	}
	DateTime(Timestamp ts) {
		struct tm *tblock;
		tblock = localtime(&ts.ts);
		tm_sec = tblock->tm_sec;
		tm_min = tblock->tm_min;
		tm_hour = tblock->tm_hour;
		tm_mday = tblock->tm_mday;
		tm_mon = tblock->tm_mon;
		tm_year = tblock->tm_year;
		tm_wday = tblock->tm_wday;
		tm_yday = tblock->tm_yday;
		tm_isdst = tblock->tm_isdst;
	}
private:
	int tm_sec;			/* Seconds.	[0-60] (1 leap second) */
	int tm_min;			/* Minutes.	[0-59] */
	int tm_hour;		/* Hours.	[0-23] */
	int tm_mday;		/* Day.		[1-31] */
	int tm_mon;			/* Month.	[0-11] */
	int tm_year;		/* Year	- 1900.  */
	int tm_wday;		/* Day of week.	[0-6] */
	int tm_yday;		/* Days in year.[0-365]	*/
	int tm_isdst;		/* DST.		[-1/0/1]*/
};


#endif // GUARD_TimeStamp_h__
