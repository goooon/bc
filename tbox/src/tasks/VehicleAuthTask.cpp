#include "./VehicleAuthTask.h"
#include "./BCMessage.h"
#include "../inc/Vehicle.h"
#include "../inc/Application.h"

VehicleAuthTask::VehicleAuthTask() :Task(APPID_AUTHENTICATION, true)
{
	VehicleAuthTask* p = this;
}

Task* VehicleAuthTask::Create()
{
	return bc_new VehicleAuthTask();
}

void VehicleAuthTask::doTask()
{
	fire.update();
	for (;;) {
		ThreadEvent::WaitResult wr = msgQueue.wait(500);
		if (wr == ThreadEvent::TimeOut) {
			Timestamp now;
			if (fire < now) {
				reqAuth();
				fire.update(Config::getInstance().getAuthRetryInterval());
			}
		}
		else if (wr == ThreadEvent::EventOk) {
			AppEvent::Type e;
			u32 param1;
			u32 param2;
			void* data;
			if (msgQueue.out(e, param1, param2, data)) {
				if (e == AppEvent::NetStateChanged) {
					if (param1 == 0) {
						LOG_I("VehicleAuthTask Exit By Net Disconnected");
						return;
					}
				}
				else if (e == AppEvent::MqttStateChanged) {
					if (param2 == MqttClient::Disconnected) {
						LOG_I("VehicleAuthTask Exit By Mqtt Disconnected");
						return;
					}
				}
				else if (e == AppEvent::PackageArrived)
				{
					if (param1 == Package::Mqtt && data != 0)
					{
						BCPackage pkg((BCPackage*)data);
						BCMessage msg(0);
						BCMessage m = pkg.nextMessage(msg);
						if (m.getApplicationId() == APPID_AUTHENTICATION)
						{
							if (m.msg) {

								BCMessage::Index idx;
								Identity at;
								if (idx = m.getFirstElement(&at)) {
									Config::getInstance().setAuthToken(at.token.dw);
								}
								else {
									LOG_E("Auth failed with imcomplete data");
									PostEvent(AppEvent::AutoEvent, Vehicle::AuthIdentity, Vehicle::Unauthed, 0);
									return;
								}
								TimeStamp ts;
								if (idx = m.getNextElement(&ts, idx)) {}
								else {
									LOG_E("Auth failed with imcomplete data");
									PostEvent(AppEvent::AutoEvent,Vehicle::AuthIdentity, Vehicle::Unauthed, 0);
									return;
								}
								ErrorElement ee;
								if (idx = m.getNextElement(&ee, idx)) {
									if (ee.errorcode == 0) {
										LOG_I("Vehicle::Authed with Identity 0x%x(%u) at %d-%d-%d %d:%d:%d", at.token, at.token, 1900 + ts.year, ts.month, ts.day, ts.hour, ts.min, ts.sec);
										PostEvent(AppEvent::AutoEvent, Vehicle::AuthIdentity, Vehicle::Authed, 0);
									}
									else {
										LOG_E("Auth failed whit errcode : %d", ee.errorcode);
										PostEvent(AppEvent::AutoEvent, Vehicle::AuthIdentity, Vehicle::Unauthed, 0);
									}
								}
								else {
									LOG_E("Auth failed with imcomplete data");
									PostEvent(AppEvent::AutoEvent, Vehicle::AuthIdentity, Vehicle::Unauthed, 0);
									return;
								}
							}
						}
						return;
					}
				}
				else {

				}
			}
		}
		else {
			PostEvent(AppEvent::AutoEvent, Vehicle::AuthIdentity, Vehicle::Unauthed, 0);
			LOG_E("taskMessage.wait(5000) failed %d", wr);
		}
	}
}

void VehicleAuthTask::reqAuth()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 0, 0);
	msg.appendVehicleDesc();
	msg.appendAuthentication();
	msg.appendTimeStamp();

	LOG_I("req Auth ...");
	if (!pkg.post(Config::getInstance().pub_topic, 2, 5000)) {
		LOG_E("req Auth failed");
	}
	else {
		PostEvent(AppEvent::AutoEvent, Vehicle::AuthIdentity, Vehicle::Authing, 0);
	}
}

