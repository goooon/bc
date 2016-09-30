#ifndef MQTT_GUARD_RemoteUnlockTask_h__
#define MQTT_GUARD_RemoteUnlockTask_h__

#include "./Task.h"
class RemoteUnlockTask : public Task {
public:
	RemoteUnlockTask(u16 appId, u8 sessionId):Task(appId,sessionId){}
	virtual void run()override
	{
		LOG_I("RemoteUnlockTask(%d,%d) run...",applicationId,sessionId);
		e.wait(5000);
		LOG_I("RemoteUnlockTask(%d,%d) finished", applicationId, sessionId);
		bc_del this;
	}
	virtual bool handlePackage(void* data, int len) {
		e.post();
		return false;
	}
private:
	ThreadEvent e;
};
#endif // GUARD_RemoteUnlockTask_h__
