#include "../../inc/util/TimeStamp.h"
#include <time.h>
#if BC_TARGET == BC_TARGET_WIN
#include <windows.h>
#endif
void Timestamp::update()
{
#if BC_TARGET == BC_TARGET_LINUX
	//gettimeofday(&tm, NULL);
	//ms = tm.tv_sec * 1000 + tm.tv_usec / 1000;

	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	ts = (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);

#elif BC_TARGET == BC_TARGET_WIN
	ts = GetTickCount();
#endif
}

void Timestamp::update(TimeVal milliseconds)
{
	update();
	ts += milliseconds;
}
