#include "./MqttConnTask.h"
#include "../inc/Application.h"
#undef TAG
#define TAG "VehicleAuthTask"
MqttConnTask::MqttConnTask():Task(APPID_MQTT_CONNECT, true) {}
void MqttConnTask::doTask()
{
	for (;;) {
		ThreadEvent::WaitResult wr = waitForEvent(Config::getInstance().getMqttReConnInterval());
		if (wr == ThreadEvent::TimeOut) {
			if (Application::getInstance().isNetConnected()) {
				if (Application::getInstance().isMqttConnected())return;
				Application::getInstance().reConnectMqtt();
			}
			else {
				return;
			}
		}
		else if (wr == ThreadEvent::EventOk) {
			MessageQueue::Args args;
			if (msgQueue.out(args)) {
				if (args.e == AppEvent::AbortTasks) {
					if (APPID_MQTT_CONNECT == args.param1) {
						return;
					}
				}
			}
		}
	}
}