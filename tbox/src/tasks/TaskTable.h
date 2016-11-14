#ifndef GUARD_TaskTable_h__
#define GUARD_TaskTable_h__
#include "../inc/Task.h"

#define APPID_TEST -1
#define APPID_AUTHENTICATION     0
#define APPID_ACQUIRE_CONFIG     7
#define APPID_VKEY_ACTIVITION    1
#define APPID_VKEY_DEACTIVITION  2
#define APPID_VKEY_IGNITION      3
#define APPID_STATE_UNIGNITION_VK    4
#define APPID_STATE_UNIGNITION_NTF	5
#define APPID_VKEY_UNIGNITION       6
#define APPID_STATE_IGNITION        8
#define APPID_GPS_UPLOADING_NTF	    9
#define APPID_GPS_UPLOADING  	    10
#define APPID_GPS_ABORMAL_MOVE      12
#define APPID_STATE_UPLOADING_HK    16

#define APPID_MQTT_CONNECT			0xFFFF0001
Task* TaskCreate(u16 appId, bcp_packet_t* pkg);
#endif // GUARD_TaskTable_h__
