#include "./VehicleAuthTask.h"
#include "./BCMessage.h"
#include "../inc/Vehicle.h"
void VehicleAuthTask::doTask()
{

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
			else if (e == AppEvent::PackageArrived)
			{
				if (param1 == Package::Mqtt && data != 0)
				{
					LOG_I("Vehicle::Authed");
					PostEvent(AppEvent::AutoStateChanged, Vehicle::Authed, 0, 0);
					return;
				}
			}
		}
	}
	else {
		PostEvent(AppEvent::AutoStateChanged, Vehicle::Unauthed, 0, 0);
		LOG_E("taskMessage.wait(5000) failed %d", wr);
	}
}

void VehicleAuthTask::reqAuth()
{
	LOG_I("reqAuth()");
	BCPackage pkg;
	LOG_I("appendMessage()");
	BCMessage msg = pkg.appendMessage(appID, 1, seqID);
	LOG_I("desc()");
	msg.appendVehicleDesc();
	LOG_I("desc()");
	msg.appendAuthentication();
	msg.appendTimeStamp();

	if (!pkg.post(Config::getInstance().pub_topic, 2, 5000)) {
		LOG_E("req Auth failed");
	}
	else {
		PostEvent(AppEvent::AutoStateChanged, Vehicle::Authing, 0, 0);
		LOG_I("req Auth ...");
	}
}

