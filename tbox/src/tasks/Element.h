#ifndef GUARD_Element_h__
#define GUARD_Element_h__
#include "../inc/dep.h"
#include "../inc/Mqtt.h"

#if BC_TARGET == BC_TARGET_WIN
#pragma pack(push, 1)
#endif

struct VehicleDesc
{
	u8 vin[17];	//utf-8 string
	u8 tbox_serial[20];//utf-8 string
	u8 imei[15];//utf-8 string
	u8 iccid[20];//utf-8 string
}DECL_GNU_PACKED;

struct AuthToken
{
	u8 token[4];
}DECL_GNU_PACKED;

struct ErrorElement
{
	u8 errorcode;	//ref errorcode.h
}DECL_GNU_PACKED;

struct TimeStamp
{
	u8 year;	//0 = 1900, 1 = 1991бн ranges up to 254 = 2154
	u8 month;   //1-12
	u8 day;		//1-31
	u8 hour;	//0-23
	u8 min;		//0-59
	u8 sec;     //0-59
}DECL_GNU_PACKED;

struct Authentication
{
	u8 PID[16];
}DECL_GNU_PACKED;

struct RemoteControl
{
	u8 command;
	u8 data[1];
}DECL_GNU_PACKED;

struct FuncCmdStatus
{
	u8 status;
	u8 data[1];
}DECL_GNU_PACKED;
#if BC_TARGET == BC_TARGET_WIN
#pragma pack(pop)
#endif
#endif // GUARD_Element_h__
