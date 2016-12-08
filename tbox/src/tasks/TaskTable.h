#ifndef GUARD_TaskTable_h__
#define GUARD_TaskTable_h__

#include "../inc/dep.h"
class Task;
typedef struct bcp_packet_s bcp_packet_t;
#define APPID_TEST 0xffffffff
#define APPID_AUTHENTICATION     0
#define APPID_ACQUIRE_CONFIG     7
#define APPID_VKEY_ACTIVITION    1
#define APPID_VKEY_DEACTIVITION  2
#define APPID_VKEY_IGNITION      3
#define APPID_STATE_UNIGNITION_VK    4
#define APPID_STATE_UNIGNITION_NTF	5
#define APPID_STATE_UNIGNITION_DELAY_NTF   6
#define APPID_STATE_IGNITION          8
#define APPID_GPS_UPLOADING_NTF_MOVE  9
#define APPID_GPS_UPLOADING_NTF_CONST 10
#define APPID_GPS_UPLOADING  	    11
#define APPID_GPS_ABNORMAL_MOVE     12
#define APPID_SHAKE_NTF             13

#define APPID_REMOTE_CONTROL		14
#define APPID_COLLIDE_NTF			15
#define APPID_REQUEST_STATE_UPLOADING    16
#define APPID_REMOTE_DIAG			17

#define STEPID_VKEY_IGNITION 2
#define STEPID_STATE_UNIGNITION_NTF 5
#define STEPID_STATE_UNIGNITION_DELAY_NTF 5
#define STEPID_AUTHENTICATION 0
#define STEPID_VKEY_ACTIVITION 2
#define STEPID_VKEY_DEACTIVITION 2
#define STEPID_SHAKE_NTF 2
#define STEPID_GPS_UPLOADING 2
#define STEPID_ACQUIRE_CONFIG 2
#define STEPID_STATE_IGNITION 2
#define STEPID_STATE_UNIGNITION_VK 2
#define STEPID_COLLIDE_NTF 2
#define STEPID_REMOTE_DIAG 2
#define STEPID_REMOTE_CONTROL 2
#define STEPID_REQUEST_STATE_UPLOADING 2
#define STEPID_PACKAGE_QUEUE 0
#define STEPID_GPS_UPLOADING_NTF_CONST 5
#define STEPID_VIS_GPS 0

/*
 * vechile inter-connect system
 */
#define APPID_VIS_GPS	0x8000

#define APPID_MQTT_CONNECT			0xFFFF0001
#define APPID_PACKAGE_QUEUE         0xFFFF0002
#define APPID_ACTIVE_TEST           0xFFFF0003

#define ACK_STEP_ID    3
#define NTF_STEP_ID    5
#define TSP_ACK_STEP_ID    8
Task* TaskCreate(u32 appId,u32 stepId, bcp_packet_t* pkg);
#endif // GUARD_TaskTable_h__
