#ifndef GUARD_TaskTable_h__
#define GUARD_TaskTable_h__
#include "../inc/Task.h"

#define APPID_TEST -1
#define APPID_AUTHENTICATION     0
#define APPID_VKEY_ACTIVITION    1
#define APPID_VKEY_DEACTIVITION  2
#define APPID_VKEY_IGNITION      3
#define APPID_STATE_UPLOADING_VK    4
#define APPID_STATE_UPLOADING_NTF	5
#define APPID_STATE_UPLOADING_HK    16
Task* TaskCreate(u16 appId, bcp_packet_t* pkg);
#endif // GUARD_TaskTable_h__
