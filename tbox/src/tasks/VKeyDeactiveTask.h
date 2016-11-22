#ifndef GUARD_VKeyDeactiveTask_h__
#define GUARD_VKeyDeactiveTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "../inc/Vehicle.h"
#include "./ErrorCode.h"
#include "./BCMessage.h"
#include "./TaskTable.h"

//ref http://jira.oa.beecloud.com:8090/pages/viewpage.action?pageId=3997706 a02

class VKeyDeavtiveTask : public Task {
public:
	static Task* Create(u32 appId);
	VKeyDeavtiveTask(u32 appId);
private:
	virtual void doTask()OVERRIDE;
private:
	Timestamp     expireTime;

};
#endif // GUARD_VKeyDeactiveTask_h__
