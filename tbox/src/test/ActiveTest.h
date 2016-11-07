#ifndef GUARD_RemoteUnlockTest_h__
#define GUARD_RemoteUnlockTest_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "../tasks/TaskTable.h"
class ActiveTest : public Task
{
	u64 seqId;
public:
	const static int AppId = APPID_TEST;
	ActiveTest():loop(true),Task(APPID_VKEY_ACTIVITION,false){

	}
	~ActiveTest() {
		LOG_I("RemoteUnlockTest Deleted!");
	}
	void stopTest(){
		loop = false;
		msgQueue.post(AppEvent::AbortTasks, 0, 0, 0);
	}
protected:
	virtual void doTask() { 
		if (!reqRemoteUnlock()) {
			LOG_I("reqRemoteUnlock FAILED");
		}
		else {
			LOG_I("reqRemoteUnlock Message dispatched FAILED");
		}
		ThreadEvent::WaitResult wr = waitForEvent(5000);
		if (wr == ThreadEvent::TimeOut) {
			LOG_I("reqRemoteUnlock Responsed");
		}
		else if(wr == ThreadEvent::EventOk) {
			MessageQueue::Args args;
			msgQueue.out(args);
			switch (args.e)
			{
			case AppEvent::PackageArrived:
				if (args.param1 == Package::Mqtt) {
					//what a ...
				}
				break;
			default:
				break;
			}
			LOG_I("reqRemoteUnlock Responsed");
		}
		return;
	}
private:
	bool reqRemoteUnlock();
private:
	bool loop;
};

class DeactiveTest : public Task
{
public:
	DeactiveTest(): Task(APPID_VKEY_ACTIVITION, false){}
};
#endif // GUARD_RemoteUnlockTest_h__
