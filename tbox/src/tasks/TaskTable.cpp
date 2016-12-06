#include "./TaskTable.h"
#include "../inc/Task.h"
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
	u32 launchId;
	char* name;
	Task* (*creator)(u32 appId);
};

static TaskTable tt[] = {
	{ APPID_VKEY_ACTIVITION,STEPID_VKEY_ACTIVITION,"APPID_VKEY_ACTIVITION",VKeyActiveTask::Create },
	{ APPID_VKEY_DEACTIVITION,STEPID_VKEY_DEACTIVITION,"APPID_VKEY_DEACTIVITION",VKeyDeavtiveTask::Create},
	{ APPID_AUTHENTICATION,STEPID_AUTHENTICATION,"APPID_AUTHENTICATION",VehicleAuthTask::Create},
	{ APPID_VKEY_IGNITION,STEPID_VKEY_IGNITION,"APPID_VKEY_IGNITION",VKeyReadyToIgnitionTask::Create},
	{ APPID_STATE_UNIGNITION_NTF,STEPID_STATE_UNIGNITION_NTF,"APPID_STATE_UNIGNITION_NTF",UnIgnitStateUploadTask_NTF::Create},
	{ APPID_STATE_UNIGNITION_DELAY_NTF,STEPID_STATE_UNIGNITION_DELAY_NTF,"APPID_STATE_UNIGNITION_DELAY_NTF" ,UnIgnitStateUploadTask_Delay_NTF::Create},
	{ APPID_GPS_UPLOADING,STEPID_GPS_UPLOADING,"APPID_GPS_UPLOADING",GpsUploadTask::Create},
	{ APPID_ACQUIRE_CONFIG,STEPID_ACQUIRE_CONFIG,"APPID_ACQUIRE_CONFIG",AcquireConfigTask::Create},
	{ APPID_STATE_IGNITION ,STEPID_STATE_IGNITION,"APPID_STATE_IGNITION",IgnitStateUploadTask_NTF::Create },
	{ APPID_PACKAGE_QUEUE,STEPID_PACKAGE_QUEUE,"APPID_PACKAGE_QUEUE" ,PackageQueueTask::Create},
	{ APPID_STATE_UNIGNITION_VK,STEPID_STATE_UNIGNITION_VK,"APPID_STATE_UNIGNITION_VK",StateUploadTask::Create },
	{ APPID_VIS_GPS,STEPID_VIS_GPS,"APPID_VIS_GPS", visGpsTask::Create },
	{ APPID_GPS_UPLOADING_NTF_CONST,STEPID_GPS_UPLOADING_NTF_CONST,"APPID_GPS_UPLOADING_NTF_CONST",GpsUploadTask_NTF::Create},
	{ APPID_SHAKE_NTF,STEPID_SHAKE_NTF,"APPID_SHAKE_NTF",VehicleShakeTask_NTF::Create},
	{ APPID_TEST,0,0,0}
};

Task* TaskCreate(u32 appId,u32 stepId, bcp_packet_t* pkg)
{
	TaskTable* t = &tt[0];
	while (t->idx != APPID_TEST) {
		if (t->idx == appId) {
			if (t->creator) {
				if (t->launchId != stepId) {
					LOG_I("TaskCreate() launchId(%d) != stepId(%d)",t->launchId,stepId);
					return 0;
				}
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
