#include "./VKeyUnIgnitTask.h"
#undef TAG
#define TAG "A04"
Task* VKeyUnIgnitTask::Create()
{
	return bc_new VKeyUnIgnitTask();
}

VKeyUnIgnitTask::VKeyUnIgnitTask() :Task(APPID_VKEY_UNIGNITION, true)
{
	expireTime.update(Config::getInstance().getIgntActivationTimeOut());
	LOG_I("VKeyIgnitionTask(%d,%lld) expire: %lld run...", appID, seqID, expireTime.getValue());
}

void VKeyUnIgnitTask::doTask()
{
	Timestamp ts;
	ts.update(Config::getInstance().getUnIgnitionNotifyDelay());
	for (;;) {
		ThreadEvent::WaitResult wr = waitForEvent(500);
		Timestamp now;
		if (now > ts) {
			ntfUnIgnited();
			break;
		}
	}
	return;
}

void VKeyUnIgnitTask::ntfUnIgnited()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 2, seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
	msg.appendErrorElement(ERR_SUCC);
	msg.appendFunctionStatus(0);
	if (!pkg.post(Config::getInstance().getPublishTopic(), Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut(),true)) {
		LOG_E("ntfUnIgnited() failed");
	}
}
