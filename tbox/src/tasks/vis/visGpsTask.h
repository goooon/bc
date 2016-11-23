#ifndef GUARD_VIS_GPSTask_h__
#define GUARD_VIS_GPSTask_h__

#include "../../inc/Task.h"
#include "../TaskTable.h"
class visGpsTask : public Task {
public:
	static Task* Create(u32 appId);
	visGpsTask(u32 appId);
	void printGps(bcp_packet_t *pkg);
protected:
	virtual void doTask();
};
#endif // GUARD_MqttConnTask_h__
