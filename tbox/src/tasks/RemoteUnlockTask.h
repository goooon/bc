#ifndef MQTT_GUARD_RemoteUnlockTask_h__
#define MQTT_GUARD_RemoteUnlockTask_h__

#include <stdio.h>

#if defined(WIN32) || defined(WIN64)
#else
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#define PWM_IOCTL_SET_FREQ		1
#define PWM_IOCTL_STOP			0

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "../inc/Vehicle.h"
#include "./ErrorCode.h"
#include "./BCMessage.h"
#include "./TaskTable.h"
class RemoteUnlockTask : public Task {
public:
	static Task* Create()
	{
		return bc_new RemoteUnlockTask();
	}
	RemoteUnlockTask():Task(APPID_REMOTE_UNLOCK,true),pkg(pkg){}
	
	void sendResponseError(){
	}
	
	void sendResponseUnlocked() {
	}
	void set_tips(int on) {
#if defined(WIN32) || defined(WIN64)
#else
		int fd = open("/dev/pwm", 0);
		if (fd < 0) {
			printf("open pwm_buzzer device");
			return;
		}
		if (on) {
			ioctl(fd, PWM_IOCTL_SET_FREQ, 10);
		} else {
			ioctl(fd, PWM_IOCTL_STOP);
			ioctl(fd, 2);
		}
		close(fd);
#endif
	}
	virtual void doTask()OVERRIDE
	{
		LOG_I("RemoteUnlockTask(%d,%lld) run...", appID, seqID);
		sendAck();
		set_tips(1);

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
			MessageQueue::Args args;
			if (msgQueue.out(args)) {
				if (args.e == AppEvent::Customized) {
					LOG_I("Unlock door ...");
					unlockDoor();
					sendResponseUnlocked();
				}
				else if (args.e == AppEvent::AbortTask) {
					return;
				}
			}
		}
		else {
			LOG_I("waitForKnobTrigger Error %d",wr);
			sendResponseError();
		}
		set_tips(0);
		return;
	}
	ThreadEvent::WaitResult waitForKnobTrigger(u32 millSeconds)
	{
		//prepare knob for triggering 
		//wait
		ThreadEvent::WaitResult ret = msgQueue.wait(5000);
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
	
	virtual void onEvent(AppEvent::e e, u32 param1, u32 param2, void* data)OVERRIDE
	{
		msgQueue.post(e, param1, param2, data);
	}
	void sendAck() {
		BCPackage pkg;
		BCMessage msg = pkg.appendMessage(appID, 1, seqID);
		msg.appendAck(1);
		if (!pkg.post(Config::getInstance().pub_topic, 1, 5000)) {
			LOG_E("req Auth failed");
		}
		else {
			PostEvent(AppEvent::AutoStateChanged, Vehicle::Authing, 0, 0);
		}
	}
	void sendResponseTimeOut() {
		
	}
private:
	u32         duringTime;
	
	bcp_packet_t* pkg;
};
#endif // GUARD_RemoteUnlockTask_h__
