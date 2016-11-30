#ifndef MQTT_GUARD_Task_h__
#define MQTT_GUARD_Task_h__

//#include "./Message.h"
#include "./Event.h"
#include "./Operation.h"
class MessageQueue
{
private:
	ThreadEvent event;
	EventQueue  eventArgs;
public:
	struct Args
	{
		AppEvent::Type e;
		u32 param1;
		u32 param2;
		void* data;
	};
	bool post(AppEvent::Type e, u32 param1, u32 param2, void* data);
	bool out(Args& args);
	bool out(AppEvent::Type& e, u32& param1, u32& param2, void*& data);
	ThreadEvent::WaitResult wait(u32 millSecond);
};
class base_package_t;
class Task : public Thread
{
	friend class Application;
	friend class TaskList;
public:
	Task(u16 appId,bool async);
	virtual ~Task();
	u16  getApplicationId() { return appID; }
	u64  getTspSequenceId() { return seqID; }
	
public:
	virtual bool handlePackage(bcp_packet_t* pkg);
	virtual bool handlePackage(base_package_t* pkg) { return true; }
	void handleDebug();
	ThreadEvent::WaitResult waitForEvent(u32 millSeconds);
	virtual void onEvent(AppEvent::Type e, u32 param1, u32 param2, void* data);
protected:
	//the function should be OVERRIDE by its subclass
	virtual void doTask(){return;}
	void ntfTimeOut();
	void rspAck();
	void ntfError(Operation::Result ret);
private:
	//called by Application
	virtual void run()OVERRIDE;
private:
	////list node in refList//////////////////////////////////////////////////////////////////////
	Task* prev;
	Task* next;
protected:
	static u64  sSeqID;
	u16  appID;	//ref in bcp
	u64  seqID; //ref in bcp
	bool isAsync;
	MessageQueue msgQueue;
};

#define RspError(ret) LOG_I("rspError(%d,%d) ---> TSP",appID, ret);ntfError(ret);
#define RspAck()      LOG_I("RspAck(%d) ---> TSP",appID);rspAck();
#define NtfTimeOut()  LOG_I("ntfTimeOut(%d) ---> TSP",appID);ntfTimeOut();
#endif // GUARD_Task_h__
