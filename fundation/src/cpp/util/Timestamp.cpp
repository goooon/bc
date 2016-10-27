#include "../../inc/util/TimeStamp.h"
#include <time.h>
#if BC_TARGET == BC_TARGET_WIN
#include <windows.h>
#endif
void Timestamp::update()
{
	ts = time(NULL);
}
