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
	const static int AppId = APPID_STATE_UPLOADING;
	static Task* Create();
	StateUploadTask() :Task(APPID_STATE_UPLOADING, true), pkg(pkg) {}

	void rspError(Operation::Result ret);

	void rspUnlocked();
	virtual void doTask()OVERRIDE;
	void rspAck();
	void rspTimeOut();
private:
	Timestamp   expireTime;
	bcp_packet_t* pkg;
};
#endif // GUARD_UploadStateTask_h__
