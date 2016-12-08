#ifndef GUARD_VehicleShakeTask_h__
#define GUARD_VehicleShakeTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "../inc/Vehicle.h"
#include "./ErrorCode.h"
#include "./BCMessage.h"
#include "./TaskTable.h"

//ref http://jira.oa.beecloud.com:8090/pages/viewpage.action?pageId=3997706 a11
#include "../../dep/mpu6050/main6050.h"
class VehicleShakeTask_NTF : public Task {
public:
	static Task* Create(u32 appId);
	VehicleShakeTask_NTF(u32 appId);
	bool ntfShaked();
	bool ntfCollided(u8 coltype, u8 level);
	void checkShaked();
	void checkCollide();
private:
	virtual void doTask()OVERRIDE;
	bool isShakingOk();
private:
	Timestamp     checkPoint;
	u32			  shakeTimeCounter[action_event_count];
	u32			  stableTimeCounter[action_event_count];
};
#endif // GUARD_VehicleShakeTask_h__
