#include "./VKeyDeactiveTask.h"

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

}
