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
	{//����״̬
		DEF_BIT4(channel);				//�ƹ⵱ǰ��λ0-16��16����λ
		DEF_BIT2(turning_direction);	//ת��ƣ�0 δ����1 ��ת��2 ��ת 3 reserved
		DEF_BIT1(front_lamp);			//����ǰ�� on/off
		DEF_BIT1(rear_lamp);			//���ں�� on/off
		DEF_BIT1(is_head_light_on);		//Զ���on/off
		DEF_BIT7(reserved);			    //�����Ժ�ʹ��
	}DECL_GNU_PACKED;
	struct Door	//total 3 bytes
	{//�������Ŵ򿪵�״̬
		union {
			struct {
				DEF_BIT1(lh_front_opened);     //��ǰ�� on/off
				DEF_BIT1(lh_front_valid);     //��ǰ�� on/off
				DEF_BIT1(rh_front_opened);	    //��ǰ�� on/off
				DEF_BIT1(rh_front_valid);	    //��ǰ�� on/off
				DEF_BIT1(lh_rear_opened);      //����� on/off
				DEF_BIT1(lh_rear_valid);      //����� on/off
				DEF_BIT1(rh_rear_opened);      //�Һ��� on/off
				DEF_BIT1(rh_rear_valid);       //�Һ��� on/off

				DEF_BIT1(ctl_lock_opened);	   //�п��� on/off
				DEF_BIT1(ctl_lock_valid);	   //�п��� on/off
				DEF_BIT1(hood_opened);		   //����� on/off
				DEF_BIT1(hood_valid);		   //����� on/off
				DEF_BIT1(luggage_door_opened); //���� on/off
				DEF_BIT1(luggage_door_valid);   //���� on/off
				DEF_BIT1(fuellid_opened);	    //���� on/off
				DEF_BIT1(fuellid_valid);	    //���� on/off
				DEF_BIT8(reserved1);            //�����Ժ�ʹ��
				DEF_BIT8(reserved2);            //�����Ժ�ʹ��
			}DECL_GNU_PACKED;
			struct {
				DEF_BIT2(lh_front);     //��ǰ�� on/off
				DEF_BIT2(rh_front);	    //��ǰ�� on/off
				DEF_BIT2(lh_rear);      //����� on/off
				DEF_BIT2(rh_rear);      //�Һ��� on/off
				DEF_BIT2(ctl_lock);		//�п��� on/off
				DEF_BIT2(hood);		    //����� on/off
				DEF_BIT2(luggage_door); //���� on/off
				DEF_BIT2(fuellid);	    //���� on/off
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
	{//����״̬
		union {
			struct {
				DEF_BIT1(lh_front_opened); 	//��ǰ���� on/off
				DEF_BIT1(lh_front_valid); 	//��ǰ���� on/off
				DEF_BIT1(rh_front_opened); 	//��ǰ���� on/off
				DEF_BIT1(rh_front_valid); 	//��ǰ���� on/off
				DEF_BIT1(lh_rear_opened); 		//��󳵴� on/off
				DEF_BIT1(lh_rear_valid); 		//��󳵴� on/off
				DEF_BIT1(rh_rear_opened); 		//�Һ󳵴� on/off
				DEF_BIT1(rh_rear_valid); 		//�Һ󳵴� on/off
				DEF_BIT4(sun_roof); 	//�������� ��λ 0-15 ��16����λ
				DEF_BIT4(reserved);		//�����Ժ�ʹ��
				DEF_BIT8(reserved1);    //�����Ժ�ʹ��
				DEF_BIT8(reserved2);    //�����Ժ�ʹ��
			}DECL_GNU_PACKED;
			struct {
				DEF_BIT2(lh_front); 	//��ǰ���� on/off
				DEF_BIT2(rh_front); 	//��ǰ���� on/off
				DEF_BIT2(lh_rear); 		//��󳵴� on/off
				DEF_BIT2(rh_rear); 		//�Һ󳵴� on/off
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
	{//����ʻ�����й�״̬
		DEF_BIT8(brake);	        //ɲ��̤�������С/����̤�µĳ̶ȣ�256 level
		DEF_BIT8(accelerator);      //���Ŵ�С/����̤�µĳ̶�, 256 level
		DEF_BIT3(shift_level);		//�Զ���0:N,1:P,2:R,4:D,5:S
									//�ֶ�: 0:N,1,2,3,4,5,6
		DEF_BIT1(shift_type);		//�ֶ���0���Զ���1
		DEF_BIT1(shift_valid);		//��λ�Ƿ���Ч��0����Ч��1��Ч
		DEF_BIT1(reserved);			//����
		DEF_BIT2(parking_break);	//פ���ƶ� on/off
		DEF_BIT2(ingnition);		//����״̬
		DEF_BIT6(reserved2);        //�����Ժ�ʹ��
		Pedal() {
			parking_break = TriState::Valid_On;
			ingnition = TriState::Valid_Off;
			shift_level = 1;
			LOG_A(sizeof(Pedal) == 4, "size wrong for Pedal %d",sizeof(Pedal));
		}
	}DECL_GNU_PACKED;
	struct Indicator	//total 4 bytes
	{//�Ǳ��ָʾ����
		u32 miles_per_hour; //�ٶ�
		u32 miles_total;	//�������
		u32 power_left;		//��ǰ����
		u32 reserved;
	}DECL_GNU_PACKED;
	struct Misc
	{//����
		DEF_BIT1(safe_belt);	    //��ȫ��  on/off
		DEF_BIT1(air_bag);		    //��ȫ����on/off
		DEF_BIT1(door_actived);	    //vkey���� on/off
		DEF_BIT1(door_locked);	    //����    on/off
		DEF_BIT1(window_locked);    //����    on/off
		DEF_BIT1(radio_onoff);	    //������  on/off
		DEF_BIT2(reserved1);		//�����Ժ�ʹ��
		DEF_BIT8(reserved2);		//�����Ժ�ʹ��
		DEF_BIT8(reserved3);		//�����Ժ�ʹ��
		DEF_BIT8(reserved4);		//�����Ժ�ʹ��
	}DECL_GNU_PACKED;
	struct VehicleState {
		Door door;					//����״̬
		Window window;				//����״̬
		Pedal pedal;				//��ʻ����״̬
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
		u8 level;			//�յ���λ
	}DECL_GNU_PACKED;
	struct Wiper
	{
		u8 state;			//�������λ
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
