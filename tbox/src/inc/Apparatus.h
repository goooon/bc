#ifndef GUARD_Apparatus_h__
#define GUARD_Apparatus_h__
#include "./dep.h"

#define DEF_BIT1(name) u32 name : 1;
#define DEF_BIT2(name) u32 name : 2;
#define DEF_BIT3(name) u32 name : 3;
#define DEF_BIT4(name) u32 name : 4;
#define DEF_BIT5(name) u32 name : 5;
#define DEF_BIT6(name) u32 name : 6;
#define DEF_BIT7(name) u32 name : 7;
#define DEF_BIT8(name) u32 name : 8;
#define DEF_BIT9(name) u32 name : 9;
#define DEF_BIT10(name) u32 name : 10;
#define DEF_BIT11(name) u32 name : 11;
#define DEF_BIT12(name) u32 name : 12;
#define DEF_BIT13(name) u32 name : 13;
#define DEF_BIT14(name) u32 name : 14;
#define DEF_BIT15(name) u32 name : 15;
#define DEF_BIT16(name) u32 name : 16;

#if BC_TARGET == BC_TARGET_WIN
#pragma pack(push, 1)
#endif

struct Apparatus {
	struct Lamp //total 2 bytes
	{//车灯状态
		DEF_BIT4(channel);				//灯光当前档位0-16共16个档位
		DEF_BIT2(turning_direction);	//转向灯：0 未开，1 左转，2 右转 3 reserved
		DEF_BIT1(front_lamp);			//室内前灯 on/off
		DEF_BIT1(rear_lamp);			//室内后灯 on/off
		DEF_BIT1(is_head_light_on);		//远光灯on/off
		DEF_BIT7(reserved);			    //保留以后使用
	}DECL_GNU_PACKED;
	struct Door	//total 2 bytes
	{//各个车门打开的状态
		DEF_BIT1(lh_front);     //左前门 on/off
		DEF_BIT1(rh_front);	    //右前门 on/off
		DEF_BIT1(lh_rear);      //左后门 on/off
		DEF_BIT1(rh_rear);      //右后门 on/off
		DEF_BIT1(hood);		    //引擎盖 on/off
		DEF_BIT1(luggage_door); //后备箱 on/off
		DEF_BIT1(fuellid);	    //充电口 on/off
		DEF_BIT1(reserved1);	//保留以后使用
		DEF_BIT8(reserved2);    //保留以后使用
	}DECL_GNU_PACKED;
	struct Window	//total 2 bytes
	{//车窗状态
		DEF_BIT1(lh_front); 	//左前车窗 on/off
		DEF_BIT1(rh_front); 	//右前车窗 on/off
		DEF_BIT1(lh_rear); 		//左后车窗 on/off
		DEF_BIT1(rh_rear); 		//右后车窗 on/off
		DEF_BIT4(moon_roof); 	//车顶车窗 档位 0-15 共16个档位
		DEF_BIT8(reserved);		//保留以后使用
	}DECL_GNU_PACKED;
	struct Pedal    //total 4 bytes
	{//和行驶操作有关状态
		DEF_BIT8(brake);	        //刹车踏板阻尼大小/或者踏下的程度，256 level
		DEF_BIT8(accelerator);      //油门大小/或者踏下的程度, 256 level
		DEF_BIT4(shift_level);		//变速杆 0:P,1:R,2:N,3:D,4:S
		DEF_BIT1(parking_break);	//驻车制动 on/off
		DEF_BIT3(reserved1);		//保留以后使用
		DEF_BIT8(reserved2);		//保留以后使用
	}DECL_GNU_PACKED;
	struct Indicator	//total 4 bytes
	{//仪表板指示数据
		u32 miles_per_hour; //速度
		u32 miles_total;	//总里程数
		u32 power_left;		//当前电量
		u32 reserved;
	}DECL_GNU_PACKED;
	struct Misc
	{//杂项
		DEF_BIT1(safe_belt);	    //安全带  on/off
		DEF_BIT1(air_bag);		    //安全气囊on/off
		DEF_BIT1(door_actived);	    //vkey激活 on/off
		DEF_BIT1(door_locked);	    //门锁    on/off
		DEF_BIT1(window_locked);    //车窗    on/off
		DEF_BIT1(radio_onoff);	    //收音机  on/off
		DEF_BIT2(reserved1);		//保留以后使用
		DEF_BIT8(reserved2);		//保留以后使用
		DEF_BIT8(reserved3);		//保留以后使用
		DEF_BIT8(reserved4);		//保留以后使用
	}DECL_GNU_PACKED;
	struct VehicleState {
		Door door;					//车门状态
		Window window;				//车窗状态
		Pedal pedal;				//驾驶操作状态
	}DECL_GNU_PACKED;
	struct AirCondition
	{
		u8 level;			//空调档位
	}DECL_GNU_PACKED;
	struct Wiper
	{
		u8 state;			//雨刮器档位
	}DECL_GNU_PACKED;
	void reset() {
		memset(this, 0, sizeof(Apparatus));
	}
	void refresh() {
	}
public:
	Door door;
	Window window;
	Pedal pedal;
	Indicator indicator;
	Misc misc;
	AirCondition ac;
	Wiper wiper;
}DECL_GNU_PACKED;

#if BC_TARGET == BC_TARGET_WIN
#pragma pack(pop)
#endif
#endif // GUARD_Apparatus_h__
