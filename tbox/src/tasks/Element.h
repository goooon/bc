#ifndef GUARD_Element_h__
#define GUARD_Element_h__
#include "../inc/dep.h"
#include "../inc/Mqtt.h"
struct VehicleDesc
{
	union {
		u8 length[2];
		u16 lenghth16;
	};
	u8 vin[17];	//utf-8 string
	u8 tbox_serial[20];//utf-8 string
	u8 imei[15];//utf-8 string
	u8 iccid[20];//utf-8 string
};

struct AuthToken
{
	union {
		u8 length[2];
		u16 length16;
	};
	u8 token[4];
};

struct ErrorElement
{
	union {
		u8 length[2];
		u16 length16;
	};
	u8 errorcode;	//ref errorcode.h
};

struct TimeStamp
{
	union {
		u8 length[2];
		u16 length16;
	};
	u8 year;	//0 = 1900, 1 = 1991бн ranges up to 254 = 2154
	u8 month;   //1-12
	u8 day;		//1-31
	u8 hour;	//0-23
	u8 min;		//0-59
	u8 sec;     //0-59
};

struct Authentication
{
	union {
		u8 length[2];
		u16 length16;
	};
	u8 PID[16];
};

struct RemoteControl
{
	union {
		u8 length[2];
		u16 length16;
	};
	u8 command;
	u8 data[1];
};

struct FuncCmdStatus
{
	union {
		u8 length[2];
		u16 lenght16;
	};
	u8 status;
	u8 data[1];
};

#endif // GUARD_Element_h__
