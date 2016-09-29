#ifndef MQTT_GUARD_Message_h__
#define MQTT_GUARD_Message_h__
#include "./dep.h"
struct DatagramHeader
{
	u8  sof[4];  //0x3f,0x6c,0x81,0x29
	u8  ProtocolVersion;
	u8  PackageLength;
};
struct DatagramEnd
{
	u8 crc[4];
	u8 eof[4]; //0x68,0x1a,0x5b,0x90
};
struct ApplicationHeader
{
	u8  ApplicationID[2]; //0:Reserve,1:Remote,2:GPS
	u8  ProtocolVersion;
	u8  SessionID;
	u8  SequenceID[8];
	u8  RemainLength[3];
};
//business data
struct Authentication
{
	u8 MessageLength[2];
	u8 PID[16];
};
struct TimeStamp
{
	u8 MessageLength[2]; //0=1900, 1=1991бн ranges up to 254=2154
	u8 Year;
	u8 Month;
	u8 Day;
	u8 Hour;
	u8 Minutes;
	u8 Seconds;
};
struct VehicleDescriptor
{
	u8  MessageLength[2];
	u8  VIN[17];
	u8  TBoxSerial[20];
	u8  IMEI[15];
	u8  ICCID[20];
};

struct Element
{
	u8 MessageLength[2];
	u8 message[1];
};

struct Message
{
	ApplicationHeader header;
	Element element[1];
};

enum ErrorCode
{
	Succ,
	AuthTokenError,
	VINAuthError,
	TSPAuthError,
	ProtocolError,
	CANError,
	TBOXBusy,
	CMDInvalid,
	PkgSeqError,
	CRCError,
	TaskFail,
	TaskOverTime,
	ResponseDataTooLong,
	ConfigError,
	EnvNotAllowed,
	FunctionDisabled,
	VINError,
	NoPower,
	VultageError
};
#endif // MQTT_GUARD_Message_h__