#include "./VKeyActiveTask.h"
#include "../inc/Sensor.h"
#undef TAG
#define TAG "A01"

Task* VKeyActiveTask::Create(u32 appId)
{
	return bc_new VKeyActiveTask(appId);
}
//APPID_VKEY_ACTIVITION
VKeyActiveTask::VKeyActiveTask(u32 appId) :Task(appId, true)
{
	expireTime.update(Config::getInstance().getDoorActivationTimeOut());
	LOG_I("RemoteUnlockTask(%d,%lld) expire: %lld run...", appID, seqID, expireTime.getValue());
}

void VKeyActiveTask::ntfDoorActived()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, NTF_STEP_ID, seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
	msg.appendErrorElement(ERR_SUCC);
	msg.appendFunctionStatus(0);
	if (!pkg.post(Config::getInstance().getPublishTopic(), Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut())) {
		LOG_E("sendResponseUnlocked failed");
	}
}

void VKeyActiveTask::ntfDoorOpened()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, NTF_STEP_ID, seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
	msg.appendErrorElement(ERR_SUCC);
	msg.appendFunctionStatus(0);
	if (!pkg.post(Config::getInstance().getPublishTopic(), Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut())) {
		LOG_E("sendResponseUnlocked failed");
	}
	else {
		LOG_I("ntfDoorOpened() ---> TSP");
	}
}

void VKeyActiveTask::doTask()
{
	Operation::Result ret;
	for (;;) {
		ThreadEvent::WaitResult wr = waitForEvent(500);
		if (wr == ThreadEvent::TimeOut) {
			Timestamp now;
			if (now > expireTime) {
				LOG_I("Active waiting Time Out %lld",expireTime.getValue());
				NtfTimeOut();
				Vehicle::getInstance().reqDeactiveDoor();
				return;
			}
		}
		else if (wr == ThreadEvent::EventOk) {
			MessageQueue::Args args;
			if (msgQueue.out(args)) {
				if (args.e == AppEvent::AutoEvent) {
					if (args.param1 == Vehicle::ActiveDoorResult) {
						if (!args.param2) {
							RspError(Operation::E_State);
							break;
						}
						else {
							LOG_I("Vehicle actived ...");
						}
					}
					else if (args.param1 == Vehicle::DoorOpened) {
						LOG_I("rspDoorOpened %d %d", args.param1,args.param2);
						ntfDoorOpened();
						break;
					}
					else {
						LOG_W("Unhandled Vehicle Event %d", args.param1);
					}
				}
				else if (args.e == AppEvent::AbortTasks) {
					RspError(Operation::W_Aborted);
					return;
				}
				else if (args.e == AppEvent::PackageArrived){
					if (args.param1 == Package::Mqtt) {
						//check stepid first
						BCPackage pkg(args.data);
						if (args.param2 == 2) {
							LOG_I("rspAck to TSP");
							RspAck();
							expireTime.update(Config::getInstance().getDoorActivationTimeOut());

							ret = Vehicle::getInstance().prepareActiveDoorByVKey();
							if (ret != Operation::Succ) {
								LOG_I("prepareActiveDoorByVKey() wrong %d", ret);
								return ntfError(ret);
							}

							ret = Vehicle::getInstance().reqActiveDoorByVKey();
							if (ret == Operation::Succ) {
								LOG_I("reqActiveDoorByVKey() Done %d", ret);
								return ntfError(ret);
							}
							else if (ret == Operation::S_Blocking) {
								LOG_I("reqActiveDoorByVKey() blocking...");
							}
							else {
								LOG_I("reqActiveDoorByVKey() Result(%d) ---> TSP", ret);
								return ntfError(ret);
							}
						}
						else {
							LOG_E("unknown step id %d", args.param2);
							return;
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
			RspError(Operation::E_Code);
			return;
		}
	}
	return;
}
