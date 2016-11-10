#include "./TaskTable.h"
#include "./VKeyActiveTask.h"
#include "./VKeyDeactiveTask.h"
#include "./VehicleAuthTask.h"
#include "./StateUploadTask.h"
#include "./VKeyIgnitionTask.h"
#include "./GpsUploadTask.h"
#include "./VKeyUnIgnitTask.h"
#include "../inc/dep.h"
struct TaskTable
{
	u32 idx;
	Task* (*creator)();
};

static TaskTable tt[] = {
	{ APPID_VKEY_ACTIVITION,VKeyActiveTask::Create },
	{APPID_VKEY_DEACTIVITION,VKeyDeavtiveTask::Create},
	{ APPID_AUTHENTICATION,VehicleAuthTask::Create},
	{ APPID_VKEY_IGNITION ,VKeyIgnitionTask::Create},
	{ APPID_STATE_UPLOADING_VK,StateUploadTask::Create},
	{ APPID_GPS_UPLOADING_NTF,GpsUploadTask_NTF::Create},
	{ APPID_VKEY_UNIGNITION ,VKeyUnIgnitTask::Create},
	{-1,0}
};
Task* TaskCreate(u16 appId, bcp_packet_t* pkg)
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
