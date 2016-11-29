#ifndef _MAIN6050_H_
#define _MAIN6050_H_

#include <typeinfo>
#include <stddef.h>
#include <stdlib.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

typedef enum
{
	sharpspeedup_event,
	sharpslowdown_event,
	sharpbrake_event,
	sharpturnleft_event,
	sharpturnright_event,
	bumpup_event,
	bumpdown_event,
	shake_event
}action_event;

typedef void (*process)(action_event);

//�춯��ֵ����λm/s^2
typedef struct
{
	u32 sharpsu;
	u32 sharpsd;	
	u32 sharpbrake;
	u32 sharpleft;
	u32 sharpright;
	u32 bumpup;
	u32 bumpdown;
	u32 shake;
	u32 freq;
	process proc;
}action_config;

//1��ʾ��ʼ���ɹ���0��ʾ��ʼ��ʧ��
extern int action_init(const action_config* t);
void action_deinit();

#endif

