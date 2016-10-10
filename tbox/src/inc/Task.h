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
		isAsync(async){}
	u16  getApplicationId() { return applicationId; }
	u16  getSessionId() { return sessionId; }
public:
	virtual bool handlePackage(bcp_packet_t* pkg) {
		return false;
	}
protected:
	//the function should be override by its subclass
	virtual void doTask(){
		return;
	}
private:
	//called by Application
	virtual void run()override;
private:
	////list node in refList//////////////////////////////////////////////////////////////////////
	Task* prev;
	Task* next;
	TaskList* refList;
protected:
	u16  applicationId;	//ref in bcp
	u16  sessionId;     //ref in bcp
	bool isAsync;
};
#endif // GUARD_Task_h__
