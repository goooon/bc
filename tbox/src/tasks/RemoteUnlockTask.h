#ifndef MQTT_GUARD_RemoteUnlockTask_h__
#define MQTT_GUARD_RemoteUnlockTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "../inc/Vehicle.h"
#include "./ErrorCode.h"
#include "./BCMessage.h"
#include "./TaskTable.h"

//ref http://jira.oa.beecloud.com:8090/pages/viewpage.action?pageId=3997706 a01

class RemoteUnlockTask : public Task {
public:
	const static int AppId = APPID_VKEY_ACTIVITION;
public:
	static Task* Create();
	RemoteUnlockTask();
private:
	virtual void doTask()OVERRIDE;
	void ntfDoorActived();
	void ntfDoorOpened();
	
private:
	Timestamp     expireTime;

};
#endif // GUARD_RemoteUnlockTask_h__
