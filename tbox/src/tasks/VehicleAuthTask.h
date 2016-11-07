#ifndef GUARD_VehicleAuthTask_h__
#define GUARD_VehicleAuthTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "./BCMessage.h"
#include "./TaskTable.h"
class VehicleAuthTask : public Task {
public:
	const static int AppId = APPID_AUTHENTICATION;
	VehicleAuthTask();
	static Task* Create();
protected:
	virtual void doTask();
private:
	void reqAuth();
};
#endif // GUARD_VehicleAuthTask_h__
