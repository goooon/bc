#include "./PackageQueueTask.h"
#include "./TaskTable.h"
#include "../inc/Application.h"
#undef TAG
#define TAG "AXX"
Task* PackageQueueTask::Create(u32 appId)
{
	return bc_new PackageQueueTask(appId);
}
//APPID_PACKAGE_QUEUE
PackageQueueTask::PackageQueueTask(u32 appId) :Task(appId, true)
{

}

void PackageQueueTask::doTask()
{
	PackageQueue& queue = Application::getInstance().getPackageQueue();
	u8* buf;
	u32 len;
	int i = 0;
	while (buf = queue.getNext(len)) {
		if (ThreadEvent::EventOk == MqttClient::getInstance().reqSendPackage(Config::getInstance().pub_topic, buf, len, 2, Config::getInstance().getMqttSendTimeOut())) {
			queue.out(buf, len);
			free(buf);
			i++;
		}
		else {
			break;
		}
	}
	LOG_I("PackageQueue data send %d task message",i);
}

