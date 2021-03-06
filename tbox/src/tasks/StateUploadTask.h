#ifndef GUARD_UploadStateTask_h__
#define GUARD_UploadStateTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "../inc/Vehicle.h"
#include "./ErrorCode.h"
#include "./BCMessage.h"
#include "./TaskTable.h"

//ref http://jira.oa.beecloud.com:8090/pages/viewpage.action?pageId=3997706 a04,a05,a06
class StateUploadTask : public Task {
	public:
		static Task* Create(u32 appId);
		StateUploadTask(u32 appId);
		//void rspAck();
		virtual void doTask()OVERRIDE;
		bool ntfState();
	private:
		Timestamp     expireTime;
};
class UnIgnitStateUploadTask_NTF : public Task {
public:
	static Task* Create(u32 appId);
	UnIgnitStateUploadTask_NTF(u32 appId);
	//void rspAck();
	virtual void doTask()OVERRIDE;
	bool ntfState();
private:
	Timestamp     expireTime;
};
class UnIgnitStateUploadTask_Delay_NTF : public Task {
public:
	static Task* Create(u32 appId);
	UnIgnitStateUploadTask_Delay_NTF(u32 appId) :Task(appId, true) {}
	virtual void doTask()OVERRIDE;
private:
};
class IgnitStateUploadTask_NTF : public Task {
public:
	static Task* Create(u32 appId);
	IgnitStateUploadTask_NTF(u32 appId) :Task(appId, true) {}
	virtual void doTask()OVERRIDE;
private:
};
#endif // GUARD_UploadStateTask_h__
