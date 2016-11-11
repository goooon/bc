#ifndef GUARD_AcquireConfigTask_h__
#define GUARD_AcquireConfigTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "./BCMessage.h"
#include "./TaskTable.h"
class AcquireConfigTask : public Task {
public:
	AcquireConfigTask();
	static Task* Create();
protected:
	virtual void doTask();
private:
	void reqConfig();
	void parseConfig(ConfigElement& ce);
private:
	Timestamp expire;
};
#endif // GUARD_AcquireConfigTask_h__
