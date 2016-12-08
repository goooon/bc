#include "./RequestState.h"
#include "../inc/RunTime.h"
#include "../inc/CanBus.h"
#define GET_CONDITION_TIMEOUT 10000

struct State
{
	u8 count;
	u8 args[1];
};
RequestState::RequestState(u32 appId):Task(appId,true)
{
}

Task* RequestState::Create(u32 appId)
{
	return bc_new RequestState(appId);
}

void RequestState::doTask()
{
	fire.update(GET_CONDITION_TIMEOUT);
	for (;;) {
		ThreadEvent::WaitResult wr = msgQueue.wait(500);
		if (wr == ThreadEvent::TimeOut) {
			Timestamp now;
			if (fire < now) {
				fire.update(Config::getInstance().getAuthRetryInterval());
			}
		}
		else if (wr == ThreadEvent::EventOk) {
			MessageQueue::Args args;
			while (msgQueue.out(args)) {
			}
		}
	}
}

void RequestState::parsePackage(MessageQueue::Args& args)
{
	if (args.e == AppEvent::PackageArrived)
	{
		if (args.param1 == Package::Mqtt && args.data != 0)
		{
			BCPackage pkg((BCPackage*)args.data);
			BCMessage msg(0);
			BCMessage m = pkg.nextMessage(msg);

			if (m.getApplicationId() == APPID_REQUEST_STATE_UPLOADING && m.getStepId() == 2)
			{
				rspAck(3);
				if (m.msg) {
					BCMessage::Index idx = m.checkIdentity(Config::getInstance().getAuthToken());
					if (!idx){LOG_E("check identity failed"); return;}
					idx = m.checkTimestamp(idx);
					if (!idx) { LOG_E("check timestamp failed"); return; }

					State* rs;
					if (idx = m.getNextElement((void**)&rs, idx)) {
						checkStateList(rs->count, rs->args);
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

static u8 static_table[] = { 0,1 };

bool RequestState::checkStateList(u8 len, u8* items)
{
	u8* saveItems;
	u8 count = RunTime::getInstance().getStateItems(&saveItems);
	u32 sizeNeeded = 0;
	for (int i = 0; i < len; ++i) {
		u32 itemidx = items[i];
		if (itemidx > sizeof(static_table)/sizeof(static_table[0])) {
			LOG_E("size not right %d",itemidx);
			return false;
		}
		sizeNeeded += static_table[items[i]];
	}
	u32 sendLen = sizeNeeded + len + 1;
	u8* sendData = (u8*)malloc(sendLen);
	if (sendData == 0) {
		LOG_E("memory not enough");
		return false;
	}
	
	u8* cursor = sendData + 1;
	for (int i = 0; i < len; ++i) {
		u8 itemidx = items[i];
		u8 size = static_table[itemidx];
		if (CanBus::getInstance().getStateBlocked(itemidx,size, cursor + size)) {
			sendData[0]++;
			cursor[0] = itemidx;
		}
		else {
			LOG_E("getStateBlocked failed %d", itemidx);
		}
	}
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 5, seqID);
	LOG_I("rspStateList(appId:%d,setpId:%d,seqId:%lld)", appID, 5, seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
	msg.appendErrorElement(0);
	msg.appendRawData(sendLen, sendData);
	if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut(), true)) {
		LOG_E("rspStateList failed");
		free(sendData);
		sendData = 0;
		return false;
	}
	free(sendData);
	sendData = 0;
	return true;
}
