#include "./VKeyDeactiveTask.h"
#undef TAG
#define TAG "VKeyDeavtiveTask"
Task* VKeyDeavtiveTask::Create()
{
	return bc_new VKeyDeavtiveTask();
}

VKeyDeavtiveTask::VKeyDeavtiveTask() :Task(APPID_VKEY_DEACTIVITION, true)
{
	expireTime.update(Config::getInstance().getIgntActivationTimeOut());
	LOG_I("VKeyIgnitionTask(%d,%lld) expire: %lld run...", appID, seqID, expireTime.getValue());
}

void VKeyDeavtiveTask::doTask()
{
	Operation::Result ret;
	for (;;) {
		ThreadEvent::WaitResult wr = waitForEvent(500);
		if (wr == ThreadEvent::TimeOut) {
			Timestamp now;
			if (now > expireTime) {
				LOG_I("VKeyDeavtiveTask Time Out %lld", expireTime.getValue());
				ntfTimeOut();
				return;
			}
		}
		else if (wr == ThreadEvent::EventOk) {
			MessageQueue::Args args;
			if (msgQueue.out(args)) {
				if (args.e == AppEvent::AutoEvent) {
					if (args.param1 == Vehicle::ActiveDoorResult) {
						if (!args.param2) {
							rspError(Operation::E_Door);
							break;
						}
						else {
							LOG_I("Deavtive door OK ---> TSP");
							rspError(Operation::Succ);
							break;
						}
					}
					else {
						LOG_W("Unhandled Vehicle Event %d", args.param1);
					}
				}
				else if (args.e == AppEvent::AbortTasks) {
					rspError(Operation::W_Aborted);
					return;
				}
				else if (args.e == AppEvent::PackageArrived) {
					if (args.param1 == Package::Mqtt) {
						//check stepid first
						BCPackage pkg(args.data);
						if (args.param2 == 2) {
							LOG_I("rspAck to TSP");
							rspAck();
							expireTime.update(Config::getInstance().getDoorActivationTimeOut());

							ret = Vehicle::getInstance().reqDeactiveDoor();
							if (ret != Operation::Succ) {
								LOG_I("reqDeactiveDoor() wrong %d", ret);
								return rspError(ret);
							}
							else {
								LOG_I("Deavtive door OK ---> TSP");
								rspError(Operation::Succ);
								return;
							}
						}
					}
					else {
						LOG_W("Unhandled Package %d", args.param1);
					}
				}
				else  if (args.e == AppEvent::AutoStateChanged) {
					LOG_W("Unhandled Event %d %d %d %lld", args.e, args.param1, args.param2, args.data);
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
