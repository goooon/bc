#ifndef GUARD_VehicleAuthTask_h__
#define GUARD_VehicleAuthTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "./BCMessage.h"
#include "./TaskTable.h"
class VehicleAuthTask : public Task {
public:
	const static int AppId = APPID_AUTHENTICATION;
	VehicleAuthTask() :Task(APPID_AUTHENTICATION, true) {}
	static Task* Create()
	{
		return bc_new VehicleAuthTask();
	}
protected:
	//the function should be OVERRIDE by its subclass
	virtual void doTask() { 
		reqAuth();
		ThreadEvent::WaitResult wr = msgQueue.wait(5000);
		if (wr == ThreadEvent::TimeOut) {
			LOG_E("Auth TimeOut");
			PostEvent(AppEvent::AutoStateChanged, Vehicle::Unauthed, 0, 0);
			return;
		}
		else if (wr == ThreadEvent::EventOk) {
			AppEvent::Type e;
			u32 param1;
			u32 param2;
			void* data;
			while (msgQueue.out(e, param1, param2, data)) {
				if (e == AppEvent::AutoStateChanged) {
					if (param1 == Vehicle::Authed) {
						LOG_I("auth succed with %d %d", param1, param2);
						PostEvent(e, param1, param2, data);
						break;
					}
				}
			}
		}
		else {
			PostEvent(AppEvent::AutoStateChanged, Vehicle::Unauthed, 0, 0);
			LOG_E("taskMessage.wait(5000) failed %d",wr);
		}
	}
private:
	void reqAuth() {
		LOG_I("reqAuth()");
		BCPackage pkg;
		BCMessage msg = pkg.appendMessage(appID, 1, seqID);
		msg.appendVehicleDesc();
		msg.appendAuthentication();
		msg.appendTimeStamp();

		if (!pkg.post(Config::getInstance().pub_topic, 1, 5000)) {
			LOG_E("req Auth failed");
		}
		else {
			PostEvent(AppEvent::AutoStateChanged, Vehicle::Authing, 0, 0);
		}
	}
private:
	MessageQueue msgQueue;
};
#endif // GUARD_VehicleAuthTask_h__
