#ifndef GUARD_VehicleShakeTask_h__
#define GUARD_VehicleShakeTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "../inc/Vehicle.h"
#include "./ErrorCode.h"
#include "./BCMessage.h"
#include "./TaskTable.h"

//ref http://jira.oa.beecloud.com:8090/pages/viewpage.action?pageId=3997706 a11

class VehicleShakeTask_NTF : public Task {
public:
	static Task* Create(u32 appId);
	VehicleShakeTask_NTF(u32 appId);
	bool ntfShaked();
private:
	virtual void doTask()OVERRIDE;
	bool isShakingOk();
private:
	Timestamp     checkPoint;
	u32			  shakeTimeCounter;
	u32			  stableTimeCounter;
};
#endif // GUARD_VehicleShakeTask_h__
