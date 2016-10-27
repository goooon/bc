#ifndef GUARD_TaskTable_h__
#define GUARD_TaskTable_h__
#include "../inc/Task.h"

#define APPID_TEST 0
#define APPID_REMOTE_UNLOCK 1
#define APPID_AUTHENTICATION 2

Task* TaskCreate(u16 appId, bcp_packet_t* pkg);
#endif // GUARD_TaskTable_h__
