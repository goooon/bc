#ifndef GUARD_AcquireConfigTask_h__
#define GUARD_AcquireConfigTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "./BCMessage.h"
#include "./TaskTable.h"
class AcquireConfigTask : public Task {
public:
	AcquireConfigTask(u32 appId);
	static Task* Create(u32 appId);
protected:
	virtual void doTask();
private:
	void reqConfig();
	void parseConfig(ConfigElement* ce);
private:
	Timestamp expire;
	int tryTimes;
};
#endif // GUARD_AcquireConfigTask_h__
