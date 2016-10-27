#ifndef GUARD_Apparatus_h__
#define GUARD_Apparatus_h__
#include "./dep.h"
struct Apparatus {
	struct Lamp
	{
		byte channel;				//灯光当前档位
		byte turning_direction : 2;	//转向灯：0 未开，1 左转，2 右转 3 reserved
		bool is_head_light_on : 1;	//远光灯on/off
		bool front_lamp : 1;		//室内前灯 on/off
		bool rear_lamp : 1;			//室内后灯 on/off
		u32  reserved : 15;
	}lamp;
	struct Door
	{//各个车门打开的状态
		bool lh_front : 1;  //左前门 on/off
		bool rh_front : 1;	//右前门 on/off
		bool lh_rear : 1;   //左后门 on/off
		bool rh_rear : 1;   //右后门 on/off
		bool hood : 1;		//引擎盖 on/off
		bool luggage_door : 1;//后备箱 on/off
		bool fuellid : 1;	//充电口 on/off
		bool reserved : 1;
	}door;
	struct Window
	{
		bool lh_front : 1;		//左前车窗 on/off
		bool rh_front : 1;		//右前车窗 on/off
		bool lh_rear : 1;		//左后车窗 on/off
		bool rh_rear : 1;		//右后车窗 on/off
		byte moon_roof : 4;		//车顶车窗 档位
	}window;
	struct Pedal
	{
		byte brake;	           //阻尼大小，256 level
		byte accelerator;      //油门大小 256 level
		byte shift_level;      //变速杆 P,R,N,D,S
		bool parking_break : 1;//驻车制动 on/off
		u32  reserved : 7;
	}pedal;
	struct Indication
	{
		u32 miles_per_hour; //速度
		u32 miles_total;	//总里程数
		u32 power_left;		//当前电量
	}indication;
	struct Misc
	{
		bool safe_belt : 1;		//安全带  on/off
		bool air_bag : 1;		//安全气囊on/off
		bool door_actived : 1;	//vkey激活 on/off
		bool door_locked : 1;	//门锁    on/off
		bool window_locked : 1;	//车窗    on/off
		bool radio_onoff : 1;	//收音机  on/off
		u32  reserved : 3;
	}misc;
	struct AirCondition
	{
		byte level;			//空调档位
	}air_cond;
	struct Wiper
	{
		byte state;			//雨刮器档位
	}wiper;
	void reset() {
		memset(this, 0, sizeof(Apparatus));
	}
	void refresh() {

	}
};
#endif // GUARD_Apparatus_h__
