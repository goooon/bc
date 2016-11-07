#include "./StateUploadTask.h"
#include "../inc/Application.h"
Task* StateUploadTask_NTF::Create()
{
	return bc_new StateUploadTask_NTF();
}

void StateUploadTask_NTF::doTask()
{
	if (Vehicle::getInstance().isParkState()) {
		LOG_I("ParkState OK,No need report");
		return;
	}
	LOG_I("ParkState is not Ok,notifing ...");
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 5, seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
	msg.appendVehicleState(Vehicle::getInstance().getApparatus().vehiState);
	if (!pkg.post(Config::getInstance().pub_topic, 2, 5000)) {
		LOG_E("StateUploadTask notify failed");
	}
}

bool StateUploadTask::ntfState()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 5, seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
	msg.appendVehicleState(Application::getInstance().getVehicle().getApparatus().vehiState);
	if (!pkg.post(Config::getInstance().pub_topic, 2, 5000)) {
		LOG_E("sendResponseUnlocked failed");
		return false;
	}
	return true;
}

Task* StateUploadTask::Create()
{
	Application::getInstance().getSchedule().remove(APPID_STATE_UPLOADING_NTF);
	return bc_new StateUploadTask();
}

StateUploadTask::StateUploadTask() :Task(APPID_STATE_UPLOADING_VK, true)
{
	rspAck();
	expireTime.update(Config::getInstance().getDoorActivationTimeOut());
}

void StateUploadTask::rspAck()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 3, seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
	msg.appendErrorElement(0);
	if (!pkg.post(Config::getInstance().pub_topic, 2, 5000)) {
		LOG_E("rspAck failed");
	}
	else {
		LOG_I("rspAck succed");
	}
}

void StateUploadTask::doTask()
{
	for (;;) {
		ThreadEvent::WaitResult wr = waitForEvent(500);
		if (wr == ThreadEvent::TimeOut) {
			Timestamp now;
			if (now > expireTime) {
				LOG_I("StateUploadTask Time Out %lld", expireTime.getValue());
				return;
			}
		}
		else {
			if (ntfState())return;
		}
	}
}