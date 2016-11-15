#include "./TaskTable.h"
#include "./VKeyActiveTask.h"
#include "./VKeyDeactiveTask.h"
#include "./VehicleAuthTask.h"
#include "./StateUploadTask.h"
#include "./VKeyIgnitionTask.h"
#include "./GpsUploadTask.h"
#include "./VKeyUnIgnitTask.h"
#include "./PackageQueueTask.h"
#include "../inc/dep.h"
struct TaskTable
{
	u32 idx;
	Task* (*creator)();
};

static TaskTable tt[] = {
	{ APPID_VKEY_ACTIVITION,VKeyActiveTask::Create },
	{ APPID_VKEY_DEACTIVITION,VKeyDeavtiveTask::Create},
	{ APPID_AUTHENTICATION,VehicleAuthTask::Create},
	{ APPID_VKEY_IGNITION ,VKeyIgnitionTask::Create},
	{ APPID_STATE_UNIGNITION_VK,UnIgnitStateUploadTask::Create},
	{ APPID_GPS_UPLOADING,GpsUploadTask::Create},
	{ APPID_VKEY_UNIGNITION ,VKeyUnIgnitTask::Create},
	{ APPID_STATE_IGNITION ,IgnitStateUploadTask_NTF::Create },
	{ APPID_PACKAGE_QUEUE ,PackageQueueTask::Create},
	{-1,0}
};
Task* TaskCreate(u32 appId, bcp_packet_t* pkg)
{
	TaskTable* t = &tt[0];
	while (t->creator) {
		if (t->idx == appId) {
			LOG_I("TaskCreate(%d,0x%x)", appId,  pkg);
			Task* task = (*t->creator)();
			if (pkg) { task->handlePackage(pkg); }
			return task;
		}
		t++;
	}
	LOG_E("Can't create Task %d 0x%x", appId,pkg);
	return NULL;
}
