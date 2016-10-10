#ifndef MQTT_GUARD_RemoteUnlockTask_h__
#define MQTT_GUARD_RemoteUnlockTask_h__

#include "./Task.h"
class RemoteUnlockTask : public Task {
public:
	RemoteUnlockTask(u16 appId, u8 sessionId,bool async):Task(appId,sessionId,async){}
	static void create_messages(bcp_packet_t *p, int count)
	{
		int i;
		bcp_message_t *m;

		for (i = 0; i < count; i++) {
			m = bcp_message_create(i, i + 1, i + 2, i + 3);
			bcp_message_append(p, m);
			create_elements(m, my_rnd(5) + i);
			if (i > 0 && i % 3 == 0) {
				LOG_I("deleteing %d", i);
				bcp_message_destroy(m);
			}
		}
	}
	void setRequest(u8 lock,u8 ver)
	{
		struct Lock_t
		{
			//
		};
		Lock_t t = fromRemoteLockData(lock, ver);
		bcp_pkt_t* p = bcp_send_message(versionId,applicationId,sessionId,stepID &t,sizeof(t));
		MqttHandler::reqSendPackage(p);
	}
	virtual void doTask()override
	{
		setRequest(1,1);
		LOG_I("Task(%d,%d) run...",applicationId,sessionId);
		e.wait(5000);
		LOG_I("Task(%d,%d) done", applicationId, sessionId);
		return;
	}
	virtual bool handlePackage(void* data, int len) {
		e.post();
		return false;
	}
	void sendAck() {
		u8 ack = 1;
		bcp_packet_t *pkg = bcp_packet_create(1);
		bcp_message_t *msg = bcp_message_create(applicationId,1, 2, sessionId);
		bcp_message_append(pkg, msg);
		bcp_element_t *ele = bcp_element_create(&ack, 1);
		bcp_element_append(msg, ele);

		u8* buf;
		u32 len;
		if (bcp_packet_serialize(pkg, &buf, &len) >= 0)
		{
			MqttHandler handler;
			handler.reqSendPackage(buf, len,0,5000);
		}
		bcp_packet_destroy(pkg);
	}
private:
	ThreadEvent e;
};
#endif // GUARD_RemoteUnlockTask_h__
