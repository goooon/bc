#ifndef GUARD_MqttConnTask_h__
#define GUARD_MqttConnTask_h__

#include "../../inc/Task.h"
#include "../../inc/Mqtt.h"
#include "../BCMessage.h"
#include "../TaskTable.h"
class visGpsTask : public Task {
public:
	visGpsTask();
	void printGps(bcp_packet_t *pkg);
protected:
	virtual void doTask();
};
#endif // GUARD_MqttConnTask_h__
