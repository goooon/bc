#include "./VehicleShakeTask.h"
#include "../../dep/mpu6050/main6050.h"
#include "../inc/Application.h"

Task* VehicleShakeTask_NTF::Create(u32 appId)
{
	return bc_new VehicleShakeTask_NTF(appId);
}

VehicleShakeTask_NTF::VehicleShakeTask_NTF(u32 appId):Task(appId,true)
{
	shakeTimeCounter = 0;
	stableTimeCounter = 0;
}

bool VehicleShakeTask_NTF::ntfShaked()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 2, Vehicle::getInstance().getTBoxSequenceId());
	LOG_I("ntfShaked(appId:%d,setpId:%d,seqId:%lld)", appID, 2, seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
	if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut(), true)) {
		LOG_E("ntfShaked failed");
		return false;
	}
	return true;
}

static u32 g_shaked = 0;
void Sensor6050Callback(int e)
{
	if (e == shake_event){
		g_shaked++;
	}
}

#define SHAKE_THRESHHOLD    2
#define STABLE_THRESHOLD 3
void VehicleShakeTask_NTF::doTask()
{
	action_config cfg;
	memset(&cfg, 0, sizeof(cfg));
	cfg.proc = (process)Sensor6050Callback;
	action_init(&cfg);
	checkPoint.update(Config::getInstance().getCheckShakingInterval());
	for (;;) {
		ThreadEvent::WaitResult wr = msgQueue.wait(500);
		if (wr == ThreadEvent::EventOk) {
			MessageQueue::Args args;
			while (msgQueue.out(args)) {
				
			}
		}
		if (isShakingOk()) {
			g_shaked = 0;
			shakeTimeCounter = 0;
			stableTimeCounter = 0;
			Vehicle::getInstance().setAbnormalShaking(false);
			continue;
		}
		Timestamp now;
		if (checkPoint < now) {
			checkPoint.update(Config::getInstance().getCheckShakingInterval());
			if (g_shaked) {
				shakeTimeCounter++;
				g_shaked = 0;
			}
			else {
				stableTimeCounter++;
			}
			if (shakeTimeCounter >= SHAKE_THRESHHOLD) {
				LOG_I("Enter Abnormal Shaking");
				Vehicle::getInstance().setAbnormalShaking(true);
				shakeTimeCounter = 0;
				stableTimeCounter = 0;
				g_shaked = 0;
				ntfShaked();
			}
			else if (stableTimeCounter >= STABLE_THRESHOLD) {
				if (Vehicle::getInstance().isShaking()) {
					LOG_I("Leave Abnormal Shaking");
					Vehicle::getInstance().setAbnormalShaking(false);
				}
				stableTimeCounter = 0;
				shakeTimeCounter = 0;
			}
		}
	}
	action_deinit();
}

bool VehicleShakeTask_NTF::isShakingOk()
{
	if (Vehicle::getInstance().isDriving())return false;
	if (Vehicle::getInstance().hasDoorOpened())return false;
	return true;
}
