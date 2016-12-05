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
struct TriState {
	enum {
		Invalid_Mask = 0,
		Valid_Off = 2,
		Valid_On = 3,
		Valid_Closed = 2,
		Valid_Opened = 3
	};
};
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
	struct Door	//total 3 bytes
	{//各个车门打开的状态
		union {
			struct {
				DEF_BIT1(lh_front_opened);     //左前门 on/off
				DEF_BIT1(lh_front_valid);     //左前门 on/off
				DEF_BIT1(rh_front_opened);	    //右前门 on/off
				DEF_BIT1(rh_front_valid);	    //右前门 on/off
				DEF_BIT1(lh_rear_opened);      //左后门 on/off
				DEF_BIT1(lh_rear_valid);      //左后门 on/off
				DEF_BIT1(rh_rear_opened);      //右后门 on/off
				DEF_BIT1(rh_rear_valid);       //右后门 on/off

				DEF_BIT1(ctl_lock_opened);	   //中控锁 on/off
				DEF_BIT1(ctl_lock_valid);	   //中控锁 on/off
				DEF_BIT1(hood_opened);		   //引擎盖 on/off
				DEF_BIT1(hood_valid);		   //引擎盖 on/off
				DEF_BIT1(luggage_door_opened); //后备箱 on/off
				DEF_BIT1(luggage_door_valid);   //后备箱 on/off
				DEF_BIT1(fuellid_opened);	    //充电口 on/off
				DEF_BIT1(fuellid_valid);	    //充电口 on/off
				DEF_BIT8(reserved1);            //保留以后使用
				DEF_BIT8(reserved2);            //保留以后使用
			}DECL_GNU_PACKED;
			struct {
				DEF_BIT2(lh_front);     //左前门 on/off
				DEF_BIT2(rh_front);	    //右前门 on/off
				DEF_BIT2(lh_rear);      //左后门 on/off
				DEF_BIT2(rh_rear);      //右后门 on/off
				DEF_BIT2(ctl_lock);		//中控锁 on/off
				DEF_BIT2(hood);		    //引擎盖 on/off
				DEF_BIT2(luggage_door); //后备箱 on/off
				DEF_BIT2(fuellid);	    //充电口 on/off
			};
			u32 doors;
		}DECL_GNU_PACKED;
		Door() {
			lh_front = TriState::Valid_Closed;
			rh_front = TriState::Valid_Closed;
			lh_rear = TriState::Valid_Closed;
			rh_rear = TriState::Valid_Closed;
			ctl_lock = TriState::Valid_Closed;
			hood = TriState::Valid_Closed;
			luggage_door = TriState::Valid_Closed;
			fuellid = TriState::Valid_Closed;
			LOG_A(sizeof(Door) == 4, "size wrong for Door %d",sizeof(Door));
		}
	}DECL_GNU_PACKED;
	struct Window	//total 2 bytes
	{//车窗状态
		union {
			struct {
				DEF_BIT1(lh_front_opened); 	//左前车窗 on/off
				DEF_BIT1(lh_front_valid); 	//左前车窗 on/off
				DEF_BIT1(rh_front_opened); 	//右前车窗 on/off
				DEF_BIT1(rh_front_valid); 	//右前车窗 on/off
				DEF_BIT1(lh_rear_opened); 		//左后车窗 on/off
				DEF_BIT1(lh_rear_valid); 		//左后车窗 on/off
				DEF_BIT1(rh_rear_opened); 		//右后车窗 on/off
				DEF_BIT1(rh_rear_valid); 		//右后车窗 on/off
				DEF_BIT4(sun_roof); 	//车顶车窗 档位 0-15 共16个档位
				DEF_BIT4(reserved);		//保留以后使用
				DEF_BIT8(reserved1);    //保留以后使用
				DEF_BIT8(reserved2);    //保留以后使用
			}DECL_GNU_PACKED;
			struct {
				DEF_BIT2(lh_front); 	//左前车窗 on/off
				DEF_BIT2(rh_front); 	//右前车窗 on/off
				DEF_BIT2(lh_rear); 		//左后车窗 on/off
				DEF_BIT2(rh_rear); 		//右后车窗 on/off
			}DECL_GNU_PACKED;
			u32 winds;
		};
		Window() {
			lh_front = TriState::Valid_Closed;
			rh_front = TriState::Valid_Closed;
			lh_rear = TriState::Valid_Closed;
			rh_rear = TriState::Valid_Closed;
			sun_roof = TriState::Valid_Closed;
			LOG_A(sizeof(Window) == 4, "size wrong for Window %d",sizeof(Window));
		}
	}DECL_GNU_PACKED;
	struct Pedal    //total 4 bytes
	{//和行驶操作有关状态
		DEF_BIT8(brake);	        //刹车踏板阻尼大小/或者踏下的程度，256 level
		DEF_BIT8(accelerator);      //油门大小/或者踏下的程度, 256 level
		DEF_BIT3(shift_level);		//自动：0:N,1:P,2:R,4:D,5:S
									//手动: 0:N,1,2,3,4,5,6
		DEF_BIT1(shift_type);		//手动：0，自动：1
		DEF_BIT1(shift_valid);		//档位是否有效：0，无效，1有效
		DEF_BIT1(reserved);			//保留
		DEF_BIT2(parking_break);	//驻车制动 on/off
		DEF_BIT2(ingnition);		//引擎状态
		DEF_BIT6(reserved2);        //保留以后使用
		Pedal() {
			parking_break = TriState::Valid_On;
			ingnition = TriState::Valid_Off;
			shift_level = 1;
			LOG_A(sizeof(Pedal) == 4, "size wrong for Pedal %d",sizeof(Pedal));
		}
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
		VehicleState() {
			LOG_A(sizeof(VehicleState) == sizeof(Door) + sizeof(Window) + sizeof(Pedal), "size wrong for VehicleState %d", sizeof(VehicleState));
		}
		void toPackage(VehicleState& s) {
			s = *this;
			s.door.lh_front = door.lh_front_valid ? door.lh_front : TriState::Invalid_Mask;
			s.door.rh_front = door.rh_front_valid ? door.rh_front : TriState::Invalid_Mask;
			s.door.lh_rear = door.lh_rear_valid ? door.lh_rear : TriState::Invalid_Mask;
			s.door.rh_rear = door.rh_rear_valid ? door.rh_rear : TriState::Invalid_Mask;

			s.door.ctl_lock = door.ctl_lock_valid ? door.ctl_lock : TriState::Invalid_Mask;
			s.door.hood = door.hood_valid ? door.hood : TriState::Invalid_Mask;
			s.door.luggage_door = door.luggage_door_valid ? door.luggage_door : TriState::Invalid_Mask;
			s.door.fuellid = door.fuellid_valid ? door.fuellid : TriState::Invalid_Mask;

			s.window.lh_front = window.lh_front_valid ? window.lh_front : TriState::Invalid_Mask;
			s.window.rh_front = window.rh_front_valid ? window.rh_front : TriState::Invalid_Mask;
			s.window.lh_rear = window.lh_rear_valid ? window.lh_rear : TriState::Invalid_Mask;
			s.window.rh_rear = window.rh_rear_valid ? window.rh_rear : TriState::Invalid_Mask;
		}
	}DECL_GNU_PACKED;
	struct AirCondition
	{
		u8 level;			//空调档位
	}DECL_GNU_PACKED;
	struct Wiper
	{
		u8 state;			//雨刮器档位
	}DECL_GNU_PACKED;
	void refresh() {
	}
public:
	VehicleState vehiState;
	Indicator indicator;
	Misc misc;
	AirCondition ac;
	Wiper wiper;
}DECL_GNU_PACKED;

#if BC_TARGET == BC_TARGET_WIN
#pragma pack(pop)
#endif
#endif // GUARD_Apparatus_h__
