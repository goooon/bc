#include "./VKeyIgnitionTask.h"
#undef TAG
#define TAG "A03"
Task* VKeyReadyToIgnitionTask::Create(u32 appId)
{
	return bc_new VKeyReadyToIgnitionTask(appId);
}
//APPID_VKEY_IGNITION
VKeyReadyToIgnitionTask::VKeyReadyToIgnitionTask(u32 appId) :Task(appId, true)
{
	expireTime.update(Config::getInstance().getIgntActivationTimeOut());
	LOG_I("VKeyIgnitionTask(%d,%lld) expire: %lld run...", appID, seqID, expireTime.getValue());
}

void VKeyReadyToIgnitionTask::doTask()
{
	Operation::Result ret;
	for (;;) {
		ThreadEvent::WaitResult wr = waitForEvent(500);
		if (wr == ThreadEvent::TimeOut) {
			Timestamp now;
			if (now > expireTime) {
				LOG_I("VKeyIgnitionTask waiting Time Out %lld", expireTime.getValue());
				Vehicle::getInstance().prepareVKeyIgnition(false);
				Vehicle::getInstance().reqReadyToIgnition(false);
				NtfTimeOut();
				return;
			}
		}
		else if (wr == ThreadEvent::EventOk) {
			MessageQueue::Args args;
			if (msgQueue.out(args)) {
				if (args.e == AppEvent::AutoEvent) {
					if (args.param1 == Vehicle::ActiveDoorResult) {
						if (!args.param2) {
							RspError(Operation::E_Door);
							break;
						}
					}
					else if (args.param1 == Vehicle::Ignite) {
						LOG_I("ntfIgnited() %d %d OK -------------------------", args.param1, args.param2);
						ntfIgnited();
						return;
					}
					else if (args.param1 == Vehicle::EnterReadToIgnit) {
						if (args.param2 == 1) {
							//ntfError(Operation::Succ);
							//return;
						}
						else {
							//ntfError(Operation::E_State);
							//return;
						}
					}
					else {
						LOG_W("Unhandled Vehicle::Event %d", args.param1);
					}
				}
				else if (args.e == AppEvent::AbortTasks) {
					RspError(Operation::W_Aborted);
					return;
				}
				else if (args.e == AppEvent::PackageArrived) {
					if (args.param1 == Package::Mqtt) {
						//check stepid first
						BCPackage pkg(args.data);
						if (args.param2 == 2) {
//							LOG_I("rspAck to TSP");
							RspAck();
							expireTime.update(Config::getInstance().getIgntActivationTimeOut());

							ret = Vehicle::getInstance().prepareVKeyIgnition(true);
							if (ret != Operation::Succ) {
								LOG_I("prepareVKeyIgnition() wrong %d", ret);
								return ntfError(ret);
							}
							else {
								LOG_I("prepareVKeyIgnition() OK");
							}
							ret = Vehicle::getInstance().reqReadyToIgnition(true);
							if (ret != Operation::S_Blocking) {
								return ntfError(ret);
							}
							else {
								LOG_I("reqReadyToIgnition() OK");
							}
						}
					}
					else {
						LOG_W("Unhandled Package %d", args.param1);
					}
				}
				else  if (args.e == AppEvent::AutoStateChanged) {
					//LOG_W("Unhandled Event %d %d %d %lld", args.e, args.param1, args.param2, args.data);
				}
				else {
					LOG_E("Unhandled Event %d", args.e);
				}
			}
		}
		else {
			LOG_I("Event Error %d", wr);
			RspError(Operation::E_Code);
			return;
		}
	}
	return;
}

void VKeyReadyToIgnitionTask::ntfIgnited()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, NTF_STEP_ID, seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
	msg.appendErrorElement(ERR_SUCC);
	msg.appendFunctionStatus(0);
	if (!pkg.post(Config::getInstance().getPublishTopic(), Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut(),true)) {
		LOG_E("ntfIgnited() failed");
	}
}
