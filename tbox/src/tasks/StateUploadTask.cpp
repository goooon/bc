#include "./StateUploadTask.h"
#include "../inc/Application.h"

#undef TAG
#define TAG "A04"
bool UnIgnitStateUploadTask_NTF::ntfState()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, NTF_STEP_ID, Vehicle::getInstance().getTBoxSequenceId());
	LOG_I("ntfState(appId:%d,setpId:%d,seqId:%lld)",appID,NTF_STEP_ID, Vehicle::getInstance().getTBoxSequenceId());
	msg.appendIdentity();
	msg.appendTimeStamp();
	msg.appendVehicleState(Application::getInstance().getVehicle().getApparatus().vehiState);
	if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut(),true)) {
		LOG_E("sendResponseUnlocked failed");
		return false;
	}
	return true;
}

Task* UnIgnitStateUploadTask_NTF::Create(u32 appId)
{//APPID_STATE_UNIGNITION_NTF
	Application::getInstance().getSchedule().remove(appId);
	return bc_new UnIgnitStateUploadTask_NTF(appId);
}

UnIgnitStateUploadTask_NTF::UnIgnitStateUploadTask_NTF(u32 appId) :Task(appId, true)
{
	expireTime.update(Config::getInstance().getDoorActivationTimeOut());
}

//void UnIgnitStateUploadTask::rspAck()
//{
//	BCPackage pkg;
//	BCMessage msg = pkg.appendMessage(appID, 3, seqID);
//	msg.appendIdentity();
//	msg.appendTimeStamp();
//	msg.appendErrorElement(0);
//	if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut())) {
//		LOG_E("rspAck failed");
//	}
//	else {
//		LOG_I("rspAck succed");
//	}
//}

void UnIgnitStateUploadTask_NTF::doTask()
{
	ntfState();
	/*for (;;) {
		ThreadEvent::WaitResult wr = waitForEvent(500);
		if (wr == ThreadEvent::TimeOut) {
			Timestamp now;
			if (now > expireTime) {
				LOG_I("StateUploadTask Time Out %lld", expireTime.getValue());
				return;
			}
		}
		else {
			LOG_I("ntfState()");
			if (ntfState())return;
		}
	}*/
}

#undef TAG
#define TAG "A06"
Task* UnIgnitStateUploadTask_Delay_NTF::Create(u32 appId)
{
	return bc_new UnIgnitStateUploadTask_Delay_NTF(appId);
}

void UnIgnitStateUploadTask_Delay_NTF::doTask()
{
	if (Vehicle::getInstance().hasDoorOpened()) {
		LOG_I("ParkState Door Opened,No need report");
		return;
	}
	if (Vehicle::getInstance().isParkState()) {
		LOG_I("ParkState OK,No need report");
		return;
	}
	u64 sid = Vehicle::getInstance().getTBoxSequenceId();
	LOG_I("ntfState(%d,%d,%lld),ParkState is not Ok,notifing ...",appID,NTF_STEP_ID, sid);
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, NTF_STEP_ID, sid);
	msg.appendIdentity();
	msg.appendTimeStamp();
	msg.appendVehicleState(Vehicle::getInstance().getApparatus().vehiState);
	if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut())) {
		LOG_E("StateUploadTask notify failed");
	}
}

#undef TAG
#define TAG "A0X"
Task* IgnitStateUploadTask_NTF::Create(u32 appId)
{
	return bc_new IgnitStateUploadTask_NTF(appId);
}

void IgnitStateUploadTask_NTF::doTask()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 2, seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
	//msg.appendVehicleState(Vehicle::getInstance().getApparatus().vehiState);
	if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut())) {
		LOG_E("IgnitStateUploadTask_NTF notify failed");
	}
}
#undef TAG
#define TAG "A0X"

Task* StateUploadTask::Create(u32 appId)
{
	return bc_new StateUploadTask(appId);
}
//APPID_STATE_UNIGNITION_VK
StateUploadTask::StateUploadTask(u32 appId) :Task(appId, true)
{

}

void StateUploadTask::doTask()
{
	RspAck();
	ntfState();
}

bool StateUploadTask::ntfState()
{
	LOG_I("ntfState(%d,%d)",appID,NTF_STEP_ID);
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, NTF_STEP_ID, seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
	msg.appendErrorElement(0);
	msg.appendVehicleState(Vehicle::getInstance().getApparatus().vehiState);
	if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut())) {
		LOG_E("StateUploadTask notify failed");
		return false;
	}
	return true;
}
