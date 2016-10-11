#ifndef MQTT_GUARD_Task_h__
#define MQTT_GUARD_Task_h__

#include "./Message.h"

class Task : public Thread
{
	friend class Application;
	friend class TaskList;
public:
	Task(u16 appId,u64 sessionId,bool async):
		prev(nullptr),
		next(nullptr),
		appID(appId),
		seqID(seqID),
		isAsync(async){}
	u16  getApplicationId() { return appID; }
	u16  getSessionId() { return seqID; }
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
	u16  appID;	//ref in bcp
	u64  seqID;     //ref in bcp
	bool isAsync;
};
#endif // GUARD_Task_h__
