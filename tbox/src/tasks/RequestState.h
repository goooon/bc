#ifndef GUARD_RequestState_h__
#define GUARD_RequestState_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "./BCMessage.h"
#include "./TaskTable.h"

class RequestState : public Task {
public:
	RequestState(u32 appId);
	static Task* Create(u32 appId);
protected:
	virtual void doTask();
	void parsePackage(MessageQueue::Args& args);
	bool checkStateList(u8 len, u8* items);
private:
	Timestamp fire;
};
#endif // GUARD_AirCondition_h__
