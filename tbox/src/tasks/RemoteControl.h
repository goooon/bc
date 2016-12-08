#ifndef GUARD_RemoteControl_h__
#define GUARD_RemoteControl_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "./BCMessage.h"
#include "./TaskTable.h"

class RemoteControlTask : public Task {
public:
	RemoteControlTask(u32 appId);
	static Task* Create(u32 appId);
protected:
	virtual void doTask();
private:
	Timestamp fire;
};
#endif // GUARD_RemoteControl_h__
