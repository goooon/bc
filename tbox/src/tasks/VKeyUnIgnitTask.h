#ifndef GUARD_VKeyUnIgnitTask_h__
#define GUARD_VKeyUnIgnitTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "../inc/Vehicle.h"
#include "./ErrorCode.h"
#include "./BCMessage.h"
#include "./TaskTable.h"
//ref http://jira.oa.beecloud.com:8090/pages/viewpage.action?pageId=3997706 a03
class VKeyUnIgnitTask : public Task {
public:
	static Task* Create();
	VKeyUnIgnitTask();
private:
	virtual void doTask()OVERRIDE;
protected:
	void ntfUnIgnited();
private:
	Timestamp     expireTime;
};
#endif // GUARD_VKeyUnIgnitTask_h__
