#ifndef GUARD_PackageQueueTask_h__
#define GUARD_PackageQueueTask_h__

#include "./TaskTable.h"
class PackageQueueTask : public Task {
public:
	static Task* Create(u32 appId);
	PackageQueueTask(u32 appId);
protected:
	virtual void doTask();
};
#endif // GUARD_PackageQueueTask_h__
