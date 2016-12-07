#include "./BCMessage.h"
#include "./PackageQueue.h"
bool BCPackage::storeForResend(u8* buf,u32 len)
{
	return PackageQueue::getInstance().in(buf, len);
}

bool BCMessage::appendIdentity()
{
	Identity identity;
//#if BC_TARGET == BC_TARGET_WIN
//#pragma pack(push, 1)
//#endif
//	struct IDS {
//		VehicleDesc desc;
//		Authentication ath;
//	}DECL_GNU_PACKED;
//#if BC_TARGET == BC_TARGET_WIN
//#pragma pack(pop)
//#endif
//	IDS ids;
	//AuthToken = CRC32(Vehicle Descriptor(¼û4.4.1)(VIN + TBox Serial + IMEI + ICCID) + Authentication(¼û4.4.5)(PID))
	/*UByte4 dw;
	dw.dw = calc_crc32((u8*)&ids, sizeof(ids));

	identity.token.b0 = dw.b3;
	identity.token.b1 = dw.b2;
	identity.token.b2 = dw.b1;
	identity.token.b3 = dw.b0;*/

	u32 tmp = Config::getInstance().getAuthToken();
	//LOG_I("TBOX Identity authToken calced 0x%x(%u) Recevied is 0x%x", identity.token.dw, identity.token.dw, tmp);

	identity.token.dw = tmp;
	bcp_element_t *e = bcp_element_create((u8*)&identity, sizeof(Identity));
	bcp_element_append(msg, e);
	return true;
}
