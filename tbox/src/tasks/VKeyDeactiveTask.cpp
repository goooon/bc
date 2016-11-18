#include "./VKeyDeactiveTask.h"
#undef TAG
#define TAG "A02"
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
				NtfTimeOut();
				return;
			}
		}
		else if (wr == ThreadEvent::EventOk) {
			MessageQueue::Args args;
			if (msgQueue.out(args)) {
				if (args.e == AppEvent::AutoEvent) {
					if (args.param1 == Vehicle::DeactiveDoorResult) {
						if (!args.param2) {
							RspError(Operation::E_State);
							break;
						}
						else {
							LOG_I("Deavtive door OK ---> TSP");
							RspError(Operation::Succ);
							break;
						}
					}
					else {
						LOG_W("Unhandled Vehicle Event %d", args.param1);
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
							RspAck();
							expireTime.update(Config::getInstance().getDoorDeactiveTimeOut());

							ret = Vehicle::getInstance().reqDeactiveDoor();
							if (ret == Operation::Succ) {
								LOG_I("reqDeactiveDoor() Done %d", ret);
								RspError(ret);
							}
							else if (ret == Operation::S_Blocking) {
								LOG_I("reqDeactiveDoor() blocking...");
							}
							else {
								LOG_I("reqDeactiveDoor() Result(%d) ---> TSP",ret);
								RspError(ret);
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
			RspError(Operation::E_Code);
			return;
		}
	}
	return;
}
