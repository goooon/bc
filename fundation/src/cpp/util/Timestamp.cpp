//#include "../../inc/util/Timestamp.h"
/*#include <time.h>
#if BC_TARGET == BC_TARGET_WIN
#include <windows.h>
#endif
Timestamp::Timestamp(int year, int day, int hour, int min, int mon, int sec)
{
	struct tm tss;
	tss.tm_year = year;
	tss.tm_mday = day;
	tss.tm_hour = hour;
	tss.tm_min = min;
	tss.tm_mon = mon;
	tss.tm_sec = sec;
	ts = mktime(&tss);
}

void Timestamp::update()
{
#if BC_TARGET == BC_TARGET_LINUX
	//gettimeofday(&tm, NULL);
	//ms = tm.tv_sec * 1000 + tm.tv_usec / 1000;

	struct timespec tss;
	clock_gettime(CLOCK_MONOTONIC, &tss);
	ts = (tss.tv_sec * 1000 + tss.tv_nsec / 1000000);

#elif BC_TARGET == BC_TARGET_WIN
	ts = GetTickCount();
#endif
}

void Timestamp::update(TimeVal milliseconds)
{
	update();
	ts += milliseconds;
}*/
