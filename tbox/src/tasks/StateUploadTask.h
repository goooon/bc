#ifndef GUARD_UploadStateTask_h__
#define GUARD_UploadStateTask_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
#include "../inc/Vehicle.h"
#include "./ErrorCode.h"
#include "./BCMessage.h"
#include "./TaskTable.h"

class StateUploadTask : public Task {
public:
	const static int AppId = APPID_STATE_UPLOADING;
	static Task* Create();
	StateUploadTask() :Task(APPID_STATE_UPLOADING, true), pkg(pkg) {}

	void sendResponseError(Operation::Result ret);

	void sendResponseUnlocked();
	virtual void doTask()OVERRIDE;
	void sendAck();
	void sendResponseTimeOut();
private:
	Timestamp   expireTime;
	bcp_packet_t* pkg;
};
#endif // GUARD_UploadStateTask_h__
