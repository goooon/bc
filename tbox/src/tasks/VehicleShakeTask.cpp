
#include "./VehicleShakeTask.h"

#include "../inc/Application.h"


Task* VehicleShakeTask_NTF::Create(u32 appId)
{
	return bc_new VehicleShakeTask_NTF(appId);
}

static u32 g_flag[action_event_count] = { 0,0,0,0,0,0,0,0 };

VehicleShakeTask_NTF::VehicleShakeTask_NTF(u32 appId):Task(appId,true)
{
	memset(shakeTimeCounter, 0, sizeof(shakeTimeCounter));
	memset(stableTimeCounter, 0, sizeof(stableTimeCounter));
	memset(g_flag, 0, sizeof(g_flag));
}

bool VehicleShakeTask_NTF::ntfShaked()
{
	BCPackage pkg;
	u64 seq = Vehicle::getInstance().getTBoxSequenceId();
	BCMessage msg = pkg.appendMessage(APPID_SHAKE_NTF, STEPID_SHAKE_NTF, seq);
	LOG_I("ntfShaked(appId:%d,setpId:%d,seqId:%lld)", appID, STEPID_SHAKE_NTF, seq);
	msg.appendIdentity();
	msg.appendTimeStamp();
	if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut(), true)) {
		LOG_E("ntfShaked failed");
		return false;
	}
	return true;
}

extern void RawGps2AutoLocation(Vehicle::RawGps& rawGps, AutoLocation& loc);
bool VehicleShakeTask_NTF::ntfCollided()
{
	BCPackage pkg;
	u64 seq = Vehicle::getInstance().getTBoxSequenceId();
	BCMessage msg = pkg.appendMessage(APPID_COLLIDE_NTF, STEPID_COLLIDE_NTF, seq);
	LOG_I("ntfCollided(appId:%d,setpId:%d,seqId:%lld)", APPID_COLLIDE_NTF, STEPID_COLLIDE_NTF, seq);
	msg.appendIdentity();
	msg.appendTimeStamp();
	Vehicle::RawGps gps;
	AutoLocation loc;
	RawGps2AutoLocation(gps, loc);
	Vehicle::getInstance().getGpsInfo(gps);
	msg.appendGPSData(loc);
	u8 type[2];
	type[0] = 1;
	type[1] = 1;
	msg.appendRawData(2, type);
	if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut(), true)) {
		LOG_E("ntfShaked failed");
		return false;
	}
	return true;
}

//1:急加速事件
//2 : 急减速事件
//3 : 急刹车事件
//4 : 急左转弯事件
//5 : 急右转弯事件
//6 : 上颠簸事件
//7 : 下颠簸事件
//8 : 驻停摇晃事件
//9~: 保留

void Sensor6050Callback(int e)
{
	if (e < sizeof(g_flag) / sizeof(g_flag[0])) {
		g_flag[e] ++;
	}
	else {
		LOG_W("unknown callback event %d", e);
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
		Timestamp now;
		if (checkPoint < now) {
			checkPoint.update(Config::getInstance().getCheckShakingInterval());
			checkShaked();
			checkCollide();
		}
	}
	action_deinit();
}

void VehicleShakeTask_NTF::checkShaked()
{
	if (isShakingOk()) {
		g_flag[shake_event] = 0;
		shakeTimeCounter[shake_event] = 0;
		stableTimeCounter[shake_event] = 0;
		Vehicle::getInstance().setAbnormalShaking(false);
		return;
	}
	
	if (g_flag[shake_event]) {
		shakeTimeCounter[shake_event]++;
		g_flag[shake_event] = 0;
	}
	else {
		stableTimeCounter[shake_event]++;
	}
	if (shakeTimeCounter[shake_event] >= SHAKE_THRESHHOLD) {
		LOG_I("Enter Abnormal Shaking");
		Vehicle::getInstance().setAbnormalShaking(true);
		shakeTimeCounter[shake_event] = 0;
		stableTimeCounter[shake_event] = 0;
		g_flag[shake_event] = 0;
		ntfShaked();
	}
	else if (stableTimeCounter[shake_event] >= STABLE_THRESHOLD) {
		if (Vehicle::getInstance().isShaking()) {
			LOG_I("Leave Abnormal Shaking");
			Vehicle::getInstance().setAbnormalShaking(false);
		}
		stableTimeCounter[shake_event] = 0;
		shakeTimeCounter[shake_event] = 0;
	}
}

void VehicleShakeTask_NTF::checkCollide()
{
	if (Vehicle::getInstance().isIgnited()) {
		if (g_flag[collide_event]) {
			g_flag[collide_event] = 0;
			ntfCollided();
		}
	}
}

bool VehicleShakeTask_NTF::isShakingOk()
{
	if (Vehicle::getInstance().isDriving())return true;
	if (Vehicle::getInstance().hasDoorOpened())return true;
	return false;
}
