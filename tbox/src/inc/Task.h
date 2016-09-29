#ifndef MQTT_GUARD_Task_h__
#define MQTT_GUARD_Task_h__

#include "./Message.h"

class Task
{
public:
	Task(u16 appId,u8 sessionId):applicationId(appId),sessionId(sessionId){}
	virtual bool handlePackage(void* data, int len) {
		return false;
	}
	virtual void run() {
		return;
	};
	u16 getApplicationId() { return applicationId; }
	u16 getSessionId() { return sessionId; }
private:
	u16 applicationId;
	u16 sessionId;
};
#endif // GUARD_Task_h__
