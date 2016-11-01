#ifndef GUARD_RemoteUnlockTest_h__
#define GUARD_RemoteUnlockTest_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "../tasks/TaskTable.h"
class RemoteUnlockTest : public Task
{
	u64 seqId;
public:
	const static int AppId = APPID_TEST;
	RemoteUnlockTest():loop(true),Task(APPID_VKEY_ACTIVITION,false){

	}
	~RemoteUnlockTest() {
		LOG_I("RemoteUnlockTest Deleted!");
	}
	void stopTest(){
		loop = false;
		msgQueue.post(AppEvent::AbortTasks, 0, 0, 0);
	}
protected:
	virtual void doTask() { 
		while (loop) {
			//LOG_I("reqRemoteUnlock()");
			ThreadEvent::WaitResult ret = msgQueue.wait(10000);
			if (ret == ThreadEvent::TimeOut) {
				reqRemoteUnlock();
				LOG_I("reqRemoteUnlock()");
				continue;
			}
			if (ret == ThreadEvent::EventOk) {
				MessageQueue::Args args;
				if (msgQueue.out(args)) {
					if (args.e == AppEvent::AbortTasks) {
						break;
					}
					else if (args.e == AppEvent::TestEvent) {
						if (args.param1 == 1) {
							reqRemoteUnlock();
						}
					}
				}
			}
			else {//error
				break;
			}
		}
		return;
	}
	virtual bool handlePackage(bcp_packet_t* pkg) {
		if (pkg != NULLPTR) {
			free(pkg);
		}
		LOG_I("RemoteUnlockTest received pkg");
		return true;
	}
private:
	void reqRemoteUnlock() {
		BCPackage pkg;
		BCMessage msg = pkg.appendMessage(appID, 3, seqID);
		msg.appendAck(1);
		if (!pkg.post(Config::getInstance().pub_topic, 1, 5000)) {
			LOG_E("req Auth failed");
		}
		else {
			PostEvent(AppEvent::AutoStateChanged, Vehicle::Authing, 0, 0);
		}
	}
private:
	bool loop;
	MessageQueue msgQueue;
};
#endif // GUARD_RemoteUnlockTest_h__
