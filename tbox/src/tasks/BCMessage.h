#ifndef GUARD_BCMessage_h__
#define GUARD_BCMessage_h__

#include "../inc/dep.h"
#include "./Element.h"
class BCMessage
{
public:
	BCMessage(bcp_message_t* msg):msg(msg){}
	bool appendAck(u8 ack) {
		bcp_element_t *ele = bcp_element_create(&ack, 1);
		bcp_element_append(msg, ele);
		return true;
	}
	bool appendVehicleDesc() {
		VehicleDesc desc;
		bcp_element_t *e = bcp_element_create((u8*)&desc, sizeof(VehicleDesc));
		bcp_element_append(msg, e);
		return true;
	}
	bool appendAuthToken() {
		AuthToken token;
		bcp_element_t *e = bcp_element_create((u8*)&token, sizeof(AuthToken));
		bcp_element_append(msg, e);
		return true;
	}
	bool appendErrorElement(u8 ec) {
		ErrorElement ele;
		ele.errorcode = ec;
		bcp_element_t *e = bcp_element_create((u8*)&ele, sizeof(ErrorElement));
		bcp_element_append(msg, e);
		return true;
	}
	bool appendTimeStamp() {
		TimeStamp ts;
		bcp_element_t *e = bcp_element_create((u8*)&ts, sizeof(TimeStamp));
		bcp_element_append(msg, e);
		return true;
	}
	bool appendAuthentication() {
		Authentication auth;
		bcp_element_t *e = bcp_element_create((u8*)&auth, sizeof(Authentication));
		bcp_element_append(msg, e);
		return true;
	}
	bool appendRemoteControl( u8 cmd) {
		RemoteControl rc;
		rc.command = cmd;
		bcp_element_t *e = bcp_element_create((u8*)&rc, sizeof(RemoteControl));
		bcp_element_append(msg, e);
		return true;
	}
private:
	bcp_message_t* msg;
};
class BCPackage
{
public:
	BCPackage() {
		pkg = bcp_packet_create();
	}
	BCMessage appendMessage(u16 appid,u8 stepid, u64 seqid) {
		return bcp_message_create(appid, stepid, seqid);
	}
	bool post(char* publish, int qos, int millSec) {
		u8* buf;
		u32 len;
		bool ret = false;
		if (bcp_packet_serialize(pkg, &buf, &len) >= 0)
		{
			ret = ThreadEvent::EventOk == MqttClient::getInstance().reqSendPackage(publish, buf, len, qos, millSec) ? true : false;
		}
		else {
			LOG_E("bcp_packet_serialize failed");
		}
		bcp_packet_destroy(pkg);
		pkg = NULL;
		return ret;
	}
	~BCPackage() {
		if (pkg != NULL) {
			bcp_packet_destroy(pkg);
		}
	}
private:
	bcp_packet_t* pkg;
};
#endif // GUARD_BCMessage_h__
