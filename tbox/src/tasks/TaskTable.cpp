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
	{1,RemoteUnlockTask::Create },
	{2,VehicleAuthTask::Create},
	{-1,0}
};
Task* TaskCreate(u16 appId, bcp_packet_t* pkg)
{
	TaskTable* t = &tt[0];
	while (t->creator) {
		if (t->idx == appId) {
			LOG_I("TaskCreate(%d,0x%x)", appId,  (void*)pkg);
			Task* task = (*t->creator)();
			task->handlePackage(pkg);
		}
	}
	LOG_E("Can't create Task %d 0x%x", appId,(void*)pkg);
	return NULL;
}
