#ifndef GUARD_BCMessage_h__
#define GUARD_BCMessage_h__

#include "../inc/dep.h"
#include "./Element.h"
#include "../inc/Apparatus.h"
#undef malloc
#undef free
//ref http://jira.oa.beecloud.com:8090/pages/viewpage.action?pageId=2818185
class BCMessage
{
	friend class BCPackage;
public:
	typedef void* Index;
public:
	BCMessage(bcp_message_t* msg):msg(msg){}
	bool appendAck(u8 ec) {
		appendIdentity();
		appendTimeStamp();
		appendErrorElement(ec);
		return true;
	}
	Index getFirstElement(Identity* token) {
		bcp_element_t *e = bcp_next_element(msg, 0);
		if (e && e->len == 4) {
			if (token) {
				token->token.b0 = e->data[0];
				token->token.b1 = e->data[1];
				token->token.b2 = e->data[2];
				token->token.b3 = e->data[3];
			}
			return e;
		}
		return 0;
	}
	Index getNextElement(TimeStamp* ts, Index idx) {
		if (idx == 0)return 0;
		bcp_element_t *e = bcp_next_element(msg, (bcp_element_t*)idx);
		if (e && e->len == 6) {
			if (ts) {
				ts->year = e->data[0];
				ts->month = e->data[1];
				ts->day = e->data[2];
				ts->hour = e->data[3];
				ts->min = e->data[4];
				ts->sec = e->data[5];
			}
			return e;
		}
		return 0;
	}
	Index getNextElement(ErrorElement* ee, Index idx) {
		if (idx == 0)return 0;
		bcp_element_t *e = bcp_next_element(msg, (bcp_element_t*)idx);
		if (e && e->len == 1) {
			if (ee) {
				ee->errorcode = e->data[0];
			}
			return e;
		}
		return 0;
	}
	u32 getApplicationId() {
		if (msg == 0)return -1;
		return msg->hdr.id;
	}
	bool appendVehicleDesc() {
		VehicleDesc desc;
		bcp_element_t *e = bcp_element_create((u8*)&desc, sizeof(VehicleDesc));
		bcp_element_append(msg, e);
		return true;
	}
	bool appendVehicleState(Apparatus::VehicleState& state) {
		bcp_element_t *e = bcp_element_create((u8*)&state, sizeof(Apparatus::VehicleState));
		bcp_element_append(msg, e);
		return true;
	}
	bool appendIdentity() {
		Identity identity;
//#if BC_TARGET_LINUX == BC_TARGET
//		//350262672
//		token.token[0] = 0x90;
//		token.token[1] = 0x95;
//		token.token[2] = 0xe0;
//		token.token[3] = 0x14;
//#else //-1869225964
//		token.token[0] = 0x90;
//		token.token[1] = 0x95;
//		token.token[2] = 0xe0;
//		token.token[3] = 0x14;
//#endif
		//token.token.dw = 0x9095E014;
		//
#if BC_TARGET == BC_TARGET_WIN
#pragma pack(push, 1)
#endif
		struct IDS {
			VehicleDesc desc;
			Authentication ath;
		}DECL_GNU_PACKED;
#if BC_TARGET == BC_TARGET_WIN
#pragma pack(pop)
#endif
		IDS ids;
		//AuthToken = CRC32(Vehicle Descriptor(见4.4.1)(VIN + TBox Serial + IMEI + ICCID) +
			//	Authentication(见4.4.5)(PID))
		DWord dw; 
		dw.dw = calc_crc32((u8*)&ids,sizeof(ids));
		
		identity.token.b0 = dw.b3;
		identity.token.b1 = dw.b2;
		identity.token.b2 = dw.b1;
		identity.token.b3 = dw.b0;
		
		LOG_I("TBOX Identity authToken 0x%x(%u)", identity.token.dw, identity.token.dw);

		identity.token.dw = Config::getInstance().getAuthToken();
		LOG_I("TBOX new identity 0x%x", identity.token.dw);

		bcp_element_t *e = bcp_element_create((u8*)&identity, sizeof(Identity));
		bcp_element_append(msg, e);
		return true;
	}
	//
	//ec 0:succ else ref 云蜂通信协议
	bool appendErrorElement(u8 ec) {
		ErrorElement ele;
		ele.errorcode = ec;
		bcp_element_t *e = bcp_element_create((u8*)&ele, sizeof(ErrorElement));
		bcp_element_append(msg, e);
		return true;
	}
	bool appendFunctionStatus(u8 rawdata) {
		u8 data[2];
		data[0] = 1;
		data[1] = rawdata;
		bcp_element_t *ele = bcp_element_create(data, 2);
		bcp_element_append(msg, ele);
		return true;
	}
	bool appendTimeStamp() {
		TimeStamp ts;
		time_t timer;//time_t就是long int 类型
		struct tm *tblock;
		timer = time(NULL);
		tblock = localtime(&timer);
		ts.day = tblock->tm_mday;
		ts.hour = tblock->tm_hour;
		ts.min = tblock->tm_min;
		ts.month = tblock->tm_mon;
		ts.sec = tblock->tm_sec;
		ts.year = tblock->tm_year;

		bcp_element_t *e = bcp_element_create((u8*)&ts, sizeof(TimeStamp));
		bcp_element_append(msg, e);
		return true;
	}
	bool appendGPSData(GPSData& gps) {
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
public:
	bcp_message_t* msg;
};
class BCPackage
{
public:
	BCPackage() {
		pkg = bcp_packet_create();
	}
	BCPackage(void* data) {
		pkg = (bcp_packet_t*)data;
	}
	BCMessage nextMessage(BCMessage msg) {
		if (pkg == 0)return 0;
		bcp_message_t* m;
		if ((m = bcp_next_message(pkg, msg.msg)) != NULL) {
			return m;
		}
		return 0;
	}
	BCMessage appendMessage(u16 appid,u8 stepid, u64 seqid) {
		bcp_message_t* msg = bcp_message_create(appid, stepid, seqid);
		if (msg == NULL) {
			LOG_E("bcp_message_create failed");
		}
		bcp_message_append(pkg, msg);
		return msg;
	}
	bool post(const char* publish, int qos, int millSec,bool isImportant = false) {
		u8* buf;
		u32 len;
		bool ret = false;
		if (bcp_packet_serialize(pkg, &buf, &len) >= 0)
		{
			ret = ThreadEvent::EventOk == MqttClient::getInstance().reqSendPackage(publish, buf, len, qos, millSec) ? true : false;
			if (!ret) {
				if (!storeForResend(buf, len)) {
					free(buf);
				}
			}
			else {
				free(buf);
			}
		}
		else {
			LOG_E("bcp_packet_serialize failed");
		}
		//LOG_I("bcp_packet_destroy");
		bcp_packet_destroy(pkg);
		pkg = NULL;
		return ret;
	}
	~BCPackage() {
		if (pkg != NULL) {
			bcp_packet_destroy(pkg);
		}
	}
	bool storeForResend(u8* buf,u32 len);
public:
	bcp_packet_t* pkg;
};
#endif // GUARD_BCMessage_h__
