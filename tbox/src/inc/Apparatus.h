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
				DEF_BIT2(lh_front);     //��ǰ�� on/off
				DEF_BIT2(rh_front);	    //��ǰ�� on/off
				DEF_BIT2(lh_rear);      //����� on/off
				DEF_BIT2(rh_rear);      //�Һ��� on/off
				DEF_BIT2(ctl_lock);		//�п��� on/off
				DEF_BIT2(hood);		    //����� on/off
				DEF_BIT2(luggage_door); //���� on/off
				DEF_BIT2(fuellid);	    //���� on/off
				DEF_BIT8(reserved1);    //�����Ժ�ʹ��
				DEF_BIT8(reserved2);    //�����Ժ�ʹ��
			}DECL_GNU_PACKED;
			u32 doors;
		}DECL_GNU_PACKED;
		Door() {
			lh_front = 2;
			rh_front = 2;
			lh_rear = 2;
			rh_rear = 2;
			ctl_lock = 2;
			hood = 2;
			luggage_door = 2;
			fuellid = 2;
			LOG_A(sizeof(Door) == 4, "size wrong for Door %d",sizeof(Door));
		}
	}DECL_GNU_PACKED;
	struct Window	//total 2 bytes
	{//����״̬
		union {
			struct {
				DEF_BIT2(lh_front); 	//��ǰ���� on/off
				DEF_BIT2(rh_front); 	//��ǰ���� on/off
				DEF_BIT2(lh_rear); 		//��󳵴� on/off
				DEF_BIT2(rh_rear); 		//�Һ󳵴� on/off
				DEF_BIT4(sun_roof); 	//�������� ��λ 0-15 ��16����λ
				DEF_BIT4(reserved);		//�����Ժ�ʹ��
				DEF_BIT8(reserved1);    //�����Ժ�ʹ��
				DEF_BIT8(reserved2);    //�����Ժ�ʹ��
			}DECL_GNU_PACKED;
			u32 winds;
		};
		Window() {
			lh_front = 2;
			rh_front = 2;
			lh_rear = 2;
			rh_rear = 2;
			sun_roof = 2;
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
			parking_break = 3;
			ingnition = 2;
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
