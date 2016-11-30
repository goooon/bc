#include "./TaskTable.h"
#include "./VKeyActiveTask.h"
#include "./VKeyDeactiveTask.h"
#include "./VehicleAuthTask.h"
#include "./StateUploadTask.h"
#include "./VKeyIgnitionTask.h"
#include "./GpsUploadTask.h"
#include "./VKeyUnIgnitTask.h"
#include "./PackageQueueTask.h"
#include "./vis/visGpsTask.h"
#include "./AcquireConfigTask.h"
#include "./VehicleShakeTask.h"
#include "../inc/dep.h"
struct TaskTable
{
	u32 idx;
	char* name;
	Task* (*creator)(u32 appId);
};

static TaskTable tt[] = {
	{ APPID_VKEY_ACTIVITION,"APPID_VKEY_ACTIVITION",VKeyActiveTask::Create },
	{ APPID_VKEY_DEACTIVITION,"APPID_VKEY_DEACTIVITION",VKeyDeavtiveTask::Create},
	{ APPID_AUTHENTICATION,"APPID_AUTHENTICATION",VehicleAuthTask::Create},
	{ APPID_VKEY_IGNITION,"APPID_VKEY_IGNITION",VKeyReadyToIgnitionTask::Create},
	{ APPID_STATE_UNIGNITION_NTF,"APPID_STATE_UNIGNITION_NTF",UnIgnitStateUploadTask_NTF::Create},
	{ APPID_STATE_UNIGNITION_DELAY_NTF,"APPID_STATE_UNIGNITION_DELAY_NTF" ,UnIgnitStateUploadTask_Delay_NTF::Create},
	{ APPID_GPS_UPLOADING,"APPID_GPS_UPLOADING",GpsUploadTask::Create},
	{ APPID_ACQUIRE_CONFIG,"APPID_ACQUIRE_CONFIG",0},
	{ APPID_STATE_IGNITION ,"APPID_STATE_IGNITION",IgnitStateUploadTask_NTF::Create },
	{ APPID_PACKAGE_QUEUE,"APPID_PACKAGE_QUEUE" ,PackageQueueTask::Create},
	{ APPID_STATE_UNIGNITION_VK,"APPID_STATE_UNIGNITION_VK",StateUploadTask::Create },
	{ APPID_VIS_GPS,"APPID_VIS_GPS", visGpsTask::Create },
	{ APPID_GPS_UPLOADING_NTF_CONST,"APPID_GPS_UPLOADING_NTF_CONST",GpsUploadTask_NTF::Create},
	{ APPID_SHAKE_NTF,"APPID_SHAKE_NTF",VehicleShakeTask_NTF::Create},
	{ APPID_TEST,0,0}
};

Task* TaskCreate(u32 appId, bcp_packet_t* pkg)
{
	TaskTable* t = &tt[0];
	while (t->idx != APPID_TEST) {
		if (t->idx == appId) {
			if (t->creator) {
				LOG_I("TaskCreate(%s[0x%x],0x%x)", t->name, appId, pkg);
				Task* task = (*t->creator)(appId);
				if (pkg) { task->handlePackage(pkg); }
				return task;
			}
			else {
				LOG_E("Can't create Task(%s[0x%x],0x%x)", t->name, appId, pkg);
				return 0;
			}
		}
		t++;
	}
	LOG_E("Can't create Task %d 0x%x", appId,pkg);
	return NULL;
}
