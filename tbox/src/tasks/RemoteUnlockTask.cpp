#include "./RemoteUnlockTask.h"
#include "../inc/Sensor.h"
#undef TAG
#define TAG "RemoteUnlockTask"

Task* RemoteUnlockTask::Create()
{
	return bc_new RemoteUnlockTask();
}

RemoteUnlockTask::RemoteUnlockTask() :Task(APPID_VKEY_ACTIVITION, true), pkg(pkg)
{
	expireTime.update(Config::getInstance().getDoorActivationTimeOut());
	LOG_I("RemoteUnlockTask(%d,%lld) expire: %lld run...", appID, seqID, expireTime.getValue());
	
}

void RemoteUnlockTask::rspError(Operation::Result ret)
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
	BCMessage msg = pkg.appendMessage(appID, 5, seqID);
	msg.appendErrorElement(ecode);
	msg.appendTimeStamp();
	msg.appendFunctionStatus(0);
	if (!pkg.post(Config::getInstance().pub_topic, 2, 5000)) {
		LOG_E("sendResponseError failed %d",ret);
	}
}

void RemoteUnlockTask::ntfDoorActived()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 5, seqID);
	msg.appendAuthToken();
	msg.appendTimeStamp();
	msg.appendErrorElement(ERR_SUCC);
	msg.appendFunctionStatus(0);
	if (!pkg.post(Config::getInstance().pub_topic,2, 5000)) {
		LOG_E("sendResponseUnlocked failed");
	}
}

void RemoteUnlockTask::ntfDoorOpened()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 5, seqID);
	msg.appendAuthToken();
	msg.appendTimeStamp();
	msg.appendErrorElement(ERR_SUCC);
	if (!pkg.post(Config::getInstance().pub_topic, 2, 5000)) {
		LOG_E("sendResponseUnlocked failed");
	}
}

void RemoteUnlockTask::doTask()
{
	Operation::Result ret;

	ret = Vehicle::getInstance().prepareActiveDoorByVKey();
	if (ret != Operation::Succ) {
		LOG_I("prepareActiveDoorByVKey() wrong %d", ret);
		return rspError(ret);
	}

	ret = Vehicle::getInstance().reqActiveDoorByVKey();
	if (ret != Operation::Succ) {
		LOG_I("reqActiveDoorByVKey() wrong %d", ret);
		return rspError(ret);
	}
	for (;;) {
		ThreadEvent::WaitResult wr = waitForEvent(500);
		if (wr == ThreadEvent::TimeOut) {
			Timestamp now;
			if (now > expireTime) {
				LOG_I("Unlock waiting Time Out %lld",expireTime.getValue());
				ntfDoorActived();
				Vehicle::getInstance().reqDeactiveDoor();
				return;
			}
		}
		else if (wr == ThreadEvent::EventOk) {
			MessageQueue::Args args;
			if (msgQueue.out(args)) {
				if (args.e == AppEvent::AutoEvent) {
					if (args.param1 == Vehicle::DoorActived) {
						ntfDoorActived();
					}
					else if (args.param1 == Vehicle::DoorOpened) {
						LOG_I("rspDoorOpened %d %d", args.param1,args.param2);
						ntfDoorOpened();
					}
					else {
						LOG_W("Unhandled Vehicle Event %d", args.param1);
					}
				}
				else if (args.e == AppEvent::AbortTasks) {
					rspError(Operation::W_Aborted);
					return;
				}
				else if (args.e == AppEvent::PackageArrived){
					if (args.param1 == Package::Mqtt) {
						//check stepid first
						if (args.param2 == 2) { 
							rspAck();
							expireTime.update(Config::getInstance().getDoorActivationTimeOut());
						}
						if (args.data) {
							BCPackage pkg(args.data);
						}
					}
					else{
						LOG_W("Unhandled Package %d", args.param1);
					}
				}
				else  if(args.e == AppEvent::AutoStateChanged){
					LOG_W("Unhandled Event %d %d %d %lld", args.e,args.param1,args.param2,args.data);
				}
				else {
					LOG_E("Unhandled Event %d", args.e);
				}
			}
		}
		else {
			LOG_I("waitForKnobTrigger Error %d", wr);
			rspError(Operation::E_Code);
			return;
		}
	}
	return;
}

void RemoteUnlockTask::rspAck()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 3, seqID);
	msg.appendAuthToken();
	msg.appendTimeStamp();
	msg.appendErrorElement(0);
	if (!pkg.post(Config::getInstance().pub_topic, 2, 5000)) {
		LOG_E("rspAck failed");
	}
	else {
		LOG_I("rspAck succed");
	}
}

void RemoteUnlockTask::ntfTimeOut()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 5, seqID);
	msg.appendAuthToken();
	msg.appendTimeStamp();
	msg.appendErrorElement(11);
	msg.appendFunctionStatus(0);

	if (!pkg.post(Config::getInstance().pub_topic, 2, 5000)) {
		LOG_E("req Auth failed");
	}
	else {
		//PostEvent(AppEvent::AutoStateChanged, Vehicle::Unauthed, 0, 0);
	}
}
