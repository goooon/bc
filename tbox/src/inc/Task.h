#ifndef MQTT_GUARD_Task_h__
#define MQTT_GUARD_Task_h__

#include "./Message.h"

class Task : public Thread
{
	friend class Application;
	friend class TaskList;
public:
	Task(u16 appId,u8 sessionId,bool async):
		prev(nullptr),
		next(nullptr),
		applicationId(appId),
		sessionId(sessionId),
		isAsync(async)
	{}
	virtual bool handlePackage(void* data, int len) {
		return false;
	}
	u16  getApplicationId() { return applicationId; }
	u16  getSessionId() { return sessionId; }
protected:
	virtual void doTask(){
		return;
	}
private:
	//called by Application
	virtual void run()override;
private:
	Task* prev;
	Task* next;
	TaskList* refList;
protected:
	u16  applicationId;
	u16  sessionId;
	bool isAsync;
};
#endif // GUARD_Task_h__
