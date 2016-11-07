#ifndef GUARD_VKeyIgnitionTask_h__
#define GUARD_VKeyIgnitionTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "../inc/Vehicle.h"
#include "./ErrorCode.h"
#include "./BCMessage.h"
#include "./TaskTable.h"
//ref http://jira.oa.beecloud.com:8090/pages/viewpage.action?pageId=3997706 a03
class VKeyIgnitionTask : public Task {
public:
	static Task* Create();
	VKeyIgnitionTask();
private:
	virtual void doTask()OVERRIDE;
protected:
	void ntfIgnited();
private:
	Timestamp     expireTime;
};


#endif // GUARD_VKeyIgnitionTask_h__
