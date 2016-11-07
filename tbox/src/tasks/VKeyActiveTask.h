#ifndef GUARD_VKeyActiveTask_h__
#define GUARD_VKeyActiveTask_h__


#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "../inc/Vehicle.h"
#include "./ErrorCode.h"
#include "./BCMessage.h"
#include "./TaskTable.h"

//ref http://jira.oa.beecloud.com:8090/pages/viewpage.action?pageId=3997706 a01

class VKeyActiveTask : public Task {
public:
	const static int AppId = APPID_VKEY_ACTIVITION;
public:
	static Task* Create();
	VKeyActiveTask();
private:
	virtual void doTask()OVERRIDE;
	void ntfDoorActived();
	void ntfDoorOpened();
	
private:
	Timestamp     expireTime;

};

#endif // GUARD_VKeyActiveTask_h__
