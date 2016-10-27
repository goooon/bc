#ifndef GUARD_Apparatus_h__
#define GUARD_Apparatus_h__
#include "./dep.h"
struct Apparatus {
	struct Lamp
	{
		byte channel;				//�ƹ⵱ǰ��λ
		byte turning_direction : 2;	//ת��ƣ�0 δ����1 ��ת��2 ��ת 3 reserved
		bool is_head_light_on : 1;	//Զ���on/off
		bool front_lamp : 1;		//����ǰ�� on/off
		bool rear_lamp : 1;			//���ں�� on/off
		u32  reserved : 15;
	}lamp;
	struct Door
	{//�������Ŵ򿪵�״̬
		bool lh_front : 1;  //��ǰ�� on/off
		bool rh_front : 1;	//��ǰ�� on/off
		bool lh_rear : 1;   //����� on/off
		bool rh_rear : 1;   //�Һ��� on/off
		bool hood : 1;		//����� on/off
		bool luggage_door : 1;//���� on/off
		bool fuellid : 1;	//���� on/off
		bool reserved : 1;
	}door;
	struct Window
	{
		bool lh_front : 1;		//��ǰ���� on/off
		bool rh_front : 1;		//��ǰ���� on/off
		bool lh_rear : 1;		//��󳵴� on/off
		bool rh_rear : 1;		//�Һ󳵴� on/off
		byte moon_roof : 4;		//�������� ��λ
	}window;
	struct Pedal
	{
		byte brake;	           //�����С��256 level
		byte accelerator;      //���Ŵ�С 256 level
		byte shift_level;      //���ٸ� P,R,N,D,S
		bool parking_break : 1;//פ���ƶ� on/off
		u32  reserved : 7;
	}pedal;
	struct Indication
	{
		u32 miles_per_hour; //�ٶ�
		u32 miles_total;	//�������
		u32 power_left;		//��ǰ����
	}indication;
	struct Misc
	{
		bool safe_belt : 1;		//��ȫ��  on/off
		bool air_bag : 1;		//��ȫ����on/off
		bool door_actived : 1;	//vkey���� on/off
		bool door_locked : 1;	//����    on/off
		bool window_locked : 1;	//����    on/off
		bool radio_onoff : 1;	//������  on/off
		u32  reserved : 3;
	}misc;
	struct AirCondition
	{
		byte level;			//�յ���λ
	}air_cond;
	struct Wiper
	{
		byte state;			//�������λ
	}wiper;
	void reset() {
		memset(this, 0, sizeof(Apparatus));
	}
	void refresh() {

	}
};
#endif // GUARD_Apparatus_h__
