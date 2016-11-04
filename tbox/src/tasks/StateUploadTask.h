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
	static Task* Create();
	StateUploadTask();
	void rspAck();
	virtual void doTask()OVERRIDE;
	bool ntfState();
private:
	Timestamp     expireTime;
};
class StateUploadTask_NTF : public Task {
public:
	static Task* Create();
	StateUploadTask_NTF() :Task(APPID_STATE_UPLOADING_VK, true) {}
	StateUploadTask_NTF(int appid) :Task(appid, true) {}
	virtual void doTask()OVERRIDE;
private:
	
};
#endif // GUARD_UploadStateTask_h__
