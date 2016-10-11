#ifndef MQTT_GUARD_RemoteUnlockTask_h__
#define MQTT_GUARD_RemoteUnlockTask_h__

#include "./Task.h"
#include "./Mqtt.h"
class EventHander
{
private:
	ThreadEvent event;
	EventQueue  eventArgs;
public:
	bool sendEvent(AppEvent e, u32 param1, u32 param2, void* data)
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
	ThreadEvent::WaitResult wait(u32 millSecond) {
		return event.wait(millSecond);
	}
};

class RemoteUnlockTask : public Task {
public:
	RemoteUnlockTask(u16 appId, u8 sessionId,bool async):Task(appId,sessionId,async){}
	void sendResponseError(){
	}
	void sendResponseTimeOut() {
	}
	void sendResponseUnlocked() {
	}
	virtual void doTask()override
	{
		LOG_I("RemoteUnlockTask(%d,%lld) run...", appID, seqID);

		sendAck();

		if (!isCurrentStateReady()) {
			LOG_I("Current State Not Ready For Unlock");
			return sendResponseError();
		}
		ThreadEvent::WaitResult wr = waitForKnobTrigger(duringTime);
		if (wr == ThreadEvent::TimeOut) {
			LOG_I("Unlock waiting Time Out");
			sendResponseTimeOut();
		}
		else if (wr == ThreadEvent::EventOk) {
			LOG_I("Unlock door ...");
			unlockDoor();
			sendResponseUnlocked();
		}
		else {
			LOG_I("waitForKnobTrigger Error %d",wr);
			sendResponseError();
		}
		return;
	}
	ThreadEvent::WaitResult waitForKnobTrigger(u32 millSeconds)
	{
		//prepare knob for triggering 
		//wait
		ThreadEvent::WaitResult ret = event.wait(5000);
		stopWaitForKnobTrigger();
		return ret;
	}
	void stopWaitForKnobTrigger() {

	}
	void unlockDoor() {

	}
	bool isCurrentStateReady()
	{
		return true;
	}
	void onPackageArrived() {

	}
	virtual bool handlePackage(void* data, int len) {
		duringTime = 10000;
		return event.sendEvent(AppEvent::Customized, 0, duringTime,0);
	}
	void sendAck() {
		u8 ack = 1;
		bcp_packet_t *pkg = bcp_packet_create();
		bcp_message_t *msg = bcp_message_create(appID,1, seqID);
		bcp_message_append(pkg, msg);
		bcp_element_t *ele = bcp_element_create(&ack, 1);
		bcp_element_append(msg, ele);

		u8* buf;
		u32 len;
		if (bcp_packet_serialize(pkg, &buf, &len) >= 0)
		{
			MqttClient::getInstance().reqSendPackage(buf, len,0,5000);
		}
		bcp_packet_destroy(pkg);
	}
private:
	u32         duringTime;
	EventHander event;
};
#endif // GUARD_RemoteUnlockTask_h__
