#ifndef GUARD_VehicleAuthTask_h__
#define GUARD_VehicleAuthTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"

class VehicleAuthTask : public Task {
public:
	VehicleAuthTask() :Task(1, 2, true) {}
protected:
	//the function should be override by its subclass
	virtual void doTask() { 
		reqAuth();
		ThreadEvent::WaitResult wr = msgQueue.wait(5000);
		if (wr == ThreadEvent::TimeOut) {
			LOG_E("Auth TimeOut");
			PostEvent(AppEvent::AutoStateChanged, Vehicle::Unauthed, 0, 0);
			return;
		}
		else if (wr == ThreadEvent::EventOk) {
			AppEvent::e e;
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
	virtual bool handlePackage(bcp_packet_t* pkg)override
	{
		bool succ = false;
		Task::handlePackage(pkg);
		return msgQueue.post(AppEvent::AutoStateChanged, Vehicle::Authed, 0, 0);
	}
private:
	void reqAuth() {
		LOG_I("reqAuth()");
		PostEvent(AppEvent::AutoStateChanged, Vehicle::Authing, 0, 0);
	}
private:
	MessageQueue msgQueue;
};
#endif // GUARD_VehicleAuthTask_h__
