#include "./RemoteUnlockTask.h"
#undef TAG
#define TAG "RemoteUnlockTask"

Task* RemoteUnlockTask::Create()
{
	return bc_new RemoteUnlockTask();
}

RemoteUnlockTask::RemoteUnlockTask() :Task(APPID_VKEY_ACTIVITION, true), pkg(pkg)
{

}

void RemoteUnlockTask::sendResponseError(Operation::Result ret)
{
	u8 ecode = 0;
	switch (ret)
	{
	case Operation::Succ:
		ecode = ERR_SUCC;
		break;
	case Operation::W_Aborted:
		ecode = ERR_CONDITION;
		break;
	case Operation::E_Code:
		ecode = ERR_CONDITION;
		break;
	case Operation::E_Auth:
		ecode = ERR_AUTHFAIL;
		break;
	case Operation::E_Permission:
		ecode = ERR_CONDITION;
		break;
	case Operation::E_DoorOpened:
		ecode = ERR_CONDITION;
		break;
	case Operation::E_Driving:
		ecode = ERR_CONDITION;
		break;
	case Operation::E_Brake:
		ecode = ERR_CONDITION;
		break;
	case Operation::E_Door:
		ecode = ERR_CONDITION;
		break;
	default:
		break;
	}

	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 1, seqID);
	msg.appendErrorElement(ecode);
	msg.appendTimeStamp();
	if (!pkg.post(Config::getInstance().pub_topic, 1, 5000)) {
		LOG_E("sendResponseError failed %d",ret);
	}
}

void RemoteUnlockTask::sendResponseUnlocked()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 1, seqID);
	msg.appendErrorElement(ERR_SUCC);
	msg.appendTimeStamp();
	if (!pkg.post(Config::getInstance().pub_topic, 1, 5000)) {
		LOG_E("sendResponseUnlocked failed");
	}
}

void RemoteUnlockTask::doTask()
{
	expireTime.update(Druation);
	LOG_I("RemoteUnlockTask(%d,%lld) expire: %lld run...", appID, seqID, expireTime.getValue());
	sendAck();

	Operation::Result ret;

	ret = Vehicle::getInstance().prepareActiveDoorByVKey();
	if (ret != Operation::Succ) {
		LOG_I("prepareActiveDoorByVKey() wrong %d", ret);
		return sendResponseError(ret);
	}

	ret = Vehicle::getInstance().reqActiveDoorByVKey();
	if (ret != Operation::Succ) {
		LOG_I("reqActiveDoorByVKey() wrong %d", ret);
		return sendResponseError(ret);
	}

	for (;;) {
		ThreadEvent::WaitResult wr = waitForEvent(500);
		if (wr == ThreadEvent::TimeOut) {
			Timestamp now;
			if (now > expireTime) {
				LOG_I("Unlock waiting Time Out %lld",expireTime.getValue());
				sendResponseTimeOut();
				Vehicle::getInstance().reqDeactiveDoor();
				return;
			}
		}
		else if (wr == ThreadEvent::EventOk) {
			MessageQueue::Args args;
			if (msgQueue.out(args)) {
				if (args.e == AppEvent::AutoEvent) {
					if (args.param1 == Vehicle::DoorActived) {
						
					}
					else if (args.param1 == Vehicle::DoorOpened) {
						sendResponseUnlocked();
					}
					else {
						LOG_W("Unhandled Vehicle Event %d", args.param1);
					}
				}
				else if (args.e == AppEvent::AbortTasks) {
					sendResponseError(Operation::W_Aborted);
					return;
				}
				else if (args.e == AppEvent::PackageArrived){
					if (args.param1 == Package::Mqtt) {
						expireTime.update(Druation);
					}
					else{
						LOG_W("Unhandled Package %d", args.param1);
					}
				}
				else {
					LOG_W("Unhandled Event %d", args.e);
				}
			}
		}
		else {
			LOG_I("waitForKnobTrigger Error %d", wr);
			sendResponseError(Operation::E_Code);
			return;
		}
	}
	return;
}

void RemoteUnlockTask::sendAck()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 1, seqID);
	msg.appendAck(1);
	if (!pkg.post(Config::getInstance().pub_topic, 1, 5000)) {
		LOG_E("req Auth failed");
	}
	else {
		PostEvent(AppEvent::AutoStateChanged, Vehicle::Authing, 0, 0);
	}
}

void RemoteUnlockTask::sendResponseTimeOut()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 1, seqID);
	msg.appendErrorElement(11);
	if (!pkg.post(Config::getInstance().pub_topic, 1, 5000)) {
		LOG_E("req Auth failed");
	}
	else {
		PostEvent(AppEvent::AutoStateChanged, Vehicle::Unauthed, 0, 0);
	}
}
