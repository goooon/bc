#ifndef GUARD_VehicleAuthTask_h__
#define GUARD_VehicleAuthTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "./BCMessage.h"
#include "./TaskTable.h"
class VehicleAuthTask : public Task {
public:
	const static int AppId = APPID_AUTHENTICATION;
	VehicleAuthTask() :Task(APPID_AUTHENTICATION, true) {
		VehicleAuthTask* p = this;
	}
	static Task* Create()
	{
		return bc_new VehicleAuthTask();
	}
protected:
	//the function should be OVERRIDE by its subclass
	virtual void doTask();
	virtual bool handlePackage(bcp_packet_t* pkg)OVERRIDE
	{
		//::Sleep(3000);
		msgQueue.post(AppEvent::PackageArrived, Package::Mqtt, 0, (void*)pkg);
		return true;
	}
private:
	void reqAuth();
};
#endif // GUARD_VehicleAuthTask_h__
