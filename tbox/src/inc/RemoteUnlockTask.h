#ifndef MQTT_GUARD_RemoteUnlockTask_h__
#define MQTT_GUARD_RemoteUnlockTask_h__

#include "./Task.h"
class RemoteUnlockTask : public Task {
public:
	RemoteUnlockTask(u16 appId, u8 sessionId,bool async):Task(appId,sessionId,async){}
	virtual void doTask()override
	{
		LOG_I("Task(%d,%d) run...",applicationId,sessionId);
		e.wait(5000);
		LOG_I("Task(%d,%d) done", applicationId, sessionId);
		return;
	}
	virtual bool handlePackage(void* data, int len) {
		e.post();
		return false;
	}
private:
	ThreadEvent e;
};
#endif // GUARD_RemoteUnlockTask_h__
