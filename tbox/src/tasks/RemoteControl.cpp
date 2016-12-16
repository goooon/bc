#include "./RemoteControl.h"
#include "../inc/Vehicle.h"
#undef TAG
#define TAG "远程控制"  //远程控制，包括空调，车窗，鸣笛，锁车
struct Item
{
	u8 index;
	u8 args;
};
struct ControlItems
{
	u8 count;
	Item item[1];
};
RemoteControlTask::RemoteControlTask(u32 appId):Task(appId,true)
{

}

Task* RemoteControlTask::Create(u32 appId)
{
	return bc_new RemoteControlTask(appId);
}

void RemoteControlTask::doTask()
{
	for (;;) {
		ThreadEvent::WaitResult wr = msgQueue.wait(500);
		if (wr == ThreadEvent::TimeOut) {
			Timestamp now;
			if (fire < now) {
			}
		}
		else if (wr == ThreadEvent::EventOk) {
			MessageQueue::Args args;
			while (msgQueue.out(args)) {
				parsePackage(args);
			}
		}
	}
}
void controlItems(u8 len, Item* items)
{
	int i = len;
	for (int i = 0; i < len; i ++) {
		Vehicle::getInstance().control(items->index, items->args);
		items++;
	}
};

void ntfControlResult(u8 len, Item* items) {
	int i = len;
	for (int i = 0; i < len; i++) {
		Vehicle::getInstance().getControlResult(items->index, items->args);
		items++;
	}
}

void RemoteControlTask::parsePackage(MessageQueue::Args& args)
{
	if (args.e == AppEvent::PackageArrived)
	{
		if (args.param1 == Package::Mqtt && args.data != 0)
		{
			BCPackage pkg((BCPackage*)args.data);
			BCMessage msg(0);
			BCMessage m = pkg.nextMessage(msg);

			if (m.getApplicationId() == APPID_REMOTE_CONTROL && m.getStepId() == 2)
			{
				RspAck();
				if (m.msg) {
					BCMessage::Index idx = m.checkIdentity(Config::getInstance().getAuthToken());
					if (!idx) { LOG_E("check identity failed"); return; }
					idx = m.checkTimestamp(idx);
					if (!idx) { LOG_E("check timestamp failed"); return; }

					RemoteRawData* rawData;
					ControlItems* items;
					if (idx = m.getNextElement((void**)&rawData, idx)) {
						items = (ControlItems*)&rawData->rawData[0];
						controlItems(items->count, items->item);
						ntfControlResult(items->count, items->item);
						BCPackage pkg;
						rawData->CmdSource = 1;
						BCMessage msg = pkg.appendMessage(appID, 5, seqID);
						LOG_I("rspStateList(appId:%d,setpId:%d,seqId:%lld)", appID, 5, seqID);
						msg.appendIdentity();
						msg.appendTimeStamp();
						msg.appendErrorElement(0);
						msg.appendRawData(items->count * 2 + 2, (u8*)rawData);
						if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut(), true)) {
							LOG_E("rspControl failed");
							return;
						}
						return;
					}
					else {
						LOG_E("GetConfig failed with imcomplete data");
						return;
					}
				}
			}
			return;
		}
	}
}


