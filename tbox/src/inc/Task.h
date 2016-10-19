#ifndef MQTT_GUARD_Task_h__
#define MQTT_GUARD_Task_h__

#include "./Message.h"
#include "./Event.h"

class MessageQueue
{
private:
	ThreadEvent event;
	EventQueue  eventArgs;
public:
	struct Args
	{
		AppEvent::e e;
		u32 param1;
		u32 param2;
		void* data;
	};
	bool post(AppEvent::e e, u32 param1, u32 param2, void* data)
	{
		bool ret = eventArgs.in(e, param1, param2, data);
		if (!ret) {
			LOG_E("eventArgs.in() failed");
			return false;
		}
		if (ThreadEvent::PostOk != event.post()) {
			LOG_E("event.post() failed");
			return false;
		}
		return true;
	}
	bool out(Args& args) {
		return eventArgs.out(args.e, args.param1, args.param2, args.data);
	}
	bool out(AppEvent::e& e, u32& param1, u32& param2, void*& data) {
		return eventArgs.out(e, param1, param2, data);
	}
	ThreadEvent::WaitResult wait(u32 millSecond) {
		return event.wait(millSecond);
	}
};

class Task : public Thread
{
	friend class Application;
	friend class TaskList;
public:
	Task(u16 appId,u64 sessionId,bool async):
		prev(NULLPTR),
		next(NULLPTR),
		appID(appId),
		seqID(seqID),
		isAsync(async){}
	~Task() {
		LOG_I("Task(%d,%lld) released", appID, seqID);
	}
	u16  getApplicationId() { return appID; }
	u16  getSequenceId() { return seqID; }
public:
	virtual bool handlePackage(bcp_packet_t* pkg) {
		if (pkg != NULLPTR) {
			free(pkg);
		}
		return false;
	}
	virtual void onEvent(AppEvent::e e, u32 param1, u32 param2, void* data);
protected:
	//the function should be OVERRIDE by its subclass
	virtual void doTask(){return;}
private:
	//called by Application
	virtual void run()OVERRIDE;
private:
	////list node in refList//////////////////////////////////////////////////////////////////////
	Task* prev;
	Task* next;
protected:
	u16  appID;	//ref in bcp
	u64  seqID; //ref in bcp
	bool isAsync;
};
#endif // GUARD_Task_h__
