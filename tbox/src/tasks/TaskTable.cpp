#include "./TaskTable.h"
#include "./RemoteUnlockTask.h"
#include "./VehicleAuthTask.h"
#include "../inc/dep.h"
struct TaskTable
{
	u32 idx;
	Task* (*creator)();
};

static TaskTable tt[] = {
	{ APPID_REMOTE_UNLOCK,RemoteUnlockTask::Create },
	{ APPID_AUTHENTICATION,VehicleAuthTask::Create},
	{-1,0}
};
Task* TaskCreate(u16 appId, bcp_packet_t* pkg)
{
	TaskTable* t = &tt[0];
	while (t->creator) {
		if (t->idx == appId) {
			LOG_I("TaskCreate(%d,0x%x)", appId,  pkg);
			Task* task = (*t->creator)();
			task->handlePackage(pkg);
		}
		t++;
	}
	LOG_E("Can't create Task %d 0x%x", appId,pkg);
	return nullptr;
}
