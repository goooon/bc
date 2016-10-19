#ifndef GUARD_RemoteUnlockTest_h__
#define GUARD_RemoteUnlockTest_h__

#include "../inc/Task.h"
#include "../inc/Mqtt.h"
class RemoteUnlockTest : public Task
{
	u64 seqId;
public:
	RemoteUnlockTest(u32 appid,u32 seqid):loop(true),Task(appid,seqid,false){

	}
	~RemoteUnlockTest() {
		LOG_I("RemoteUnlockTest Deleted!");
	}
	void stopTest(){
		loop = false;
		msgQueue.post(AppEvent::AbortTask, 0, 0, 0);
	}
protected:
	virtual void doTask() { 
		
		while (loop) {
			//LOG_I("reqRemoteUnlock()");
			ThreadEvent::WaitResult ret = msgQueue.wait(10000);
			if (ret == ThreadEvent::TimeOut) {
				reqRemoteUnlock();
				LOG_I("reqRemoteUnlock()");
				continue;
			}
			if (ret == ThreadEvent::EventOk) {
				MessageQueue::Args args;
				if (msgQueue.out(args)) {
					if (args.e == AppEvent::AbortTask) {
						break;
					}
					else if (args.e == AppEvent::TestEvent) {
						if (args.param1 == 1) {
							reqRemoteUnlock();
						}
					}
				}
			}
			else {//error
				break;
			}
		}
		return;
	}
private:
	void reqRemoteUnlock() {
		u8 ack = 1;
		bcp_packet_t *pkg = bcp_packet_create();
		seqId = bcp_next_seq_id();

		bcp_message_t *msg = bcp_message_create(1, 1, seqId);
		bcp_message_append(pkg, msg);
		bcp_element_t *ele = bcp_element_create(&ack, 1);
		bcp_element_append(msg, ele);

		struct TimeStamp
		{
			u8 len[2];
			u8 year;
			u8 month;
			u8 day;
			u8 hour;
			u8 min;
			u8 sec;
		}ts;
		bcp_element_t *tse = bcp_element_create((u8*)&ts, sizeof(ts));
		bcp_element_append(msg, tse);

		u8* buf;
		u32 len;
		if (bcp_packet_serialize(pkg, &buf, &len) >= 0)
		{
			Config& cfg = Config::getInstance();
			MqttClient::getInstance().reqSendPackage(cfg.pub_topic, buf, len, 0, 5000);
		}
		bcp_packet_destroy(pkg);
	}
private:
	bool loop;
	MessageQueue msgQueue;
};
#endif // GUARD_RemoteUnlockTest_h__
