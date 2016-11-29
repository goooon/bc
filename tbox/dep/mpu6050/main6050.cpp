#include "main6050.h"
#include "../../src/inc/dep.h"

#if BC_TARGET_LINUX == BC_TARGET
#include <sys/ioctl.h> 
#include <fcntl.h> 
#include <linux/i2c-dev.h> 
#include <linux/i2c.h> 
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>

#define conversion_rate 205
#define shake_threshold 3000

#define sharpsu_min 	 6
#define sharpsu_max	     10

#define sharpsd_min 	 6
#define sharpsd_max	     10

#define sharpleft_min 	 10
#define sharpleft_max	15

#define sharpright_min 	10
#define sharpright_max	15

#define bumpup_min 	   10
#define bumpup_max	   20

#define bumpdown_min 	10
#define bumpdown_max	20

#define sharpbrake_min 	12
#define sharpbrake_max	16

#define shake_min 3
#define shake_max 5

#define freq_min 10
#define freq_max 100


#define I2C_DEV "/dev/i2c-0" 
#define CHIP_ADDR 0X68 


//****************************************
// 定义MPU6050内部地址
//****************************************
#define	SMPLRT_DIV		0x19	//陀螺仪采样率，典型值：0x07(125Hz)
#define	CONFIG			0x1A	//低通滤波频率，典型值：0x06(5Hz)
#define	GYRO_CONFIG		0x1B	//陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define	ACCEL_CONFIG	0x1C	//加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)
#define	ACCEL_XOUT_H	0x3B
#define	ACCEL_XOUT_L	0x3C
#define	ACCEL_YOUT_H	0x3D
#define	ACCEL_YOUT_L	0x3E
#define	ACCEL_ZOUT_H	0x3F
#define	ACCEL_ZOUT_L	0x40
#define	TEMP_OUT_H		0x41
#define	TEMP_OUT_L		0x42
#define	GYRO_XOUT_H		0x43
#define	GYRO_XOUT_L		0x44	
#define	GYRO_YOUT_H		0x45
#define	GYRO_YOUT_L		0x46
#define	GYRO_ZOUT_H		0x47
#define	GYRO_ZOUT_L		0x48
#define	PWR_MGMT_1		0x6B	//电源管理，典型值：0x00(正常启用)
#define	WHO_AM_I		0x75	//IIC地址寄存器(默认数值0x68，只读)
#define	SlaveAddress	0xD0	//IIC写入时的地址字节数据，+1为读取

typedef enum
{
	wait_action,
	in_action
}action_state;

static int fd6050;
static action_config config;
static pthread_t thProcess;
static int stop = 0;
static action_state state;

static void* process_shock(void* data);

void write_6050(u8 reg, u8 value)
{
	struct i2c_rdwr_ioctl_data i2c;	
	struct i2c_msg msg;
	u8 buf[2];
	
	i2c.msgs = &msg;	
	/* write data to e2prom*/
	i2c.nmsgs = 1; //写只有进行一次的起动总线、就是说只发送I2C消息的次数只有一次、相反读的话我们看时序可以发现我们进行了两次的起动总线、所以我们要发两次的I2C消息的次数。
	i2c.msgs[0].len = 2;//信息长度为2，看写时序，eeprom的地址不算的，因为付给了addr，而len是指buf中的值的个数
								//BUF中值的个数要是写时序的话就是两个、包括写入的单元地址和要写入的信息 分别对应下面的BUF【0】和BUF【1】
	i2c.msgs[0].addr = CHIP_ADDR;
	i2c.msgs[0].flags = 0;//写命令
	i2c.msgs[0].buf = buf;
	i2c.msgs[0].buf[0] = reg;//信息值1 eeprom中存储单元的地址，即你要往哪写
	i2c.msgs[0].buf[1] = value;//信息值2,即你要写什么

	ioctl (fd6050, I2C_RDWR, (u32)&i2c);//好了 ，写进去吧
}

u8 read_6050(u8 reg)
{
	struct i2c_rdwr_ioctl_data i2c;	
	struct i2c_msg msg[2];
	u8 buf0[2];
	u8 buf1[2];
	
	i2c.msgs = msg;
	i2c.nmsgs = 2;//读时序要两次过程，要发两次I2C消息
	//写只有进行一次的起动总线、就是说只发送I2C消息的次数只有一次、相反读的话我们看时序可以发现我们进行了两次的起动总线、所以我们要发两次的I2C消息的次数。

	
	i2c.msgs[0].len = 1;//信息长度为1，第一次只写要读的eeprom中存储单元的地址
	i2c.msgs[0].addr = CHIP_ADDR; //器件地址
	i2c.msgs[0].flags = 0;//写命令，看读时序理解	
	i2c.msgs[0].buf = buf0;
	i2c.msgs[0].buf[0] = reg;//要写入数据的单元地址


	i2c.msgs[1].len = 1;
	i2c.msgs[1].addr = CHIP_ADDR; //器件地址
	i2c.msgs[1].flags = I2C_M_RD;//读命令
	i2c.msgs[1].buf = buf1;
	i2c.msgs[1].buf[0] = 0;//先清空要读的缓冲区

	ioctl(fd6050, I2C_RDWR, (u32)&i2c);//好了，读吧


	return i2c.msgs[1].buf[0];
}

u16 get_data(u8 reg)
{
	u8 H;
	u8 L;
	H=read_6050(reg);
	L=read_6050(reg+1);
	return (H<<8)|L;   //合成数据
}


void init_6050()
{
	u8 v;

	fd6050 = open(I2C_DEV, O_RDWR|O_SYNC);

	if(fd6050<0)
	{
		perror("open i2c device failed\r\n");
		exit(-1);
	}

	ioctl(fd6050, I2C_TIMEOUT, 2);//设置超时时间
	ioctl(fd6050, I2C_RETRIES, 1);//设置重发次数
	
	write_6050(PWR_MGMT_1, 0x80);	//解除休眠状态
	sleep(1);
	write_6050(PWR_MGMT_1, 0x00);
	write_6050(SMPLRT_DIV, 0x07);
	write_6050(CONFIG, 0x06);
	write_6050(GYRO_CONFIG, 0x18);
	//1<<0|3<<3
	write_6050(ACCEL_CONFIG, 0x19);
}


int action_init(const action_config* t)
{
	if(!t)
	{
		return 0;
	}

	if(t->sharpbrake<sharpbrake_min)
	{
		config.sharpbrake = sharpbrake_min;
	}
	else if(t->sharpbrake>sharpbrake_max)
	{
		config.sharpbrake = sharpbrake_max;
	}
	else
	{
		config.sharpbrake = t->sharpbrake; 
	}

	if(t->sharpsu<sharpsu_min)
	{
		config.sharpsu = sharpsu_min;
	}
	else if(t->sharpsu>sharpsu_max)
	{
		config.sharpsu = sharpsu_max;
	}
	else
	{
		config.sharpsu = t->sharpsu; 
	}


	if(t->sharpsd<sharpsd_min)
	{
		config.sharpsd = sharpsd_min;
	}
	else if(t->sharpsd>sharpsd_max)
	{
		config.sharpsd = sharpsd_max;
	}
	else
	{
		config.sharpsd = t->sharpsd; 
	}

	
	if(t->sharpleft<sharpleft_min)
	{
		config.sharpleft = sharpleft_min;
	}
	else if(t->sharpleft>sharpleft_max)
	{
		config.sharpleft = sharpleft_max;
	}
	else
	{
		config.sharpleft = t->sharpleft; 
	}
	

	if(t->sharpright<sharpright_min)
	{
		config.sharpright = sharpright_min;
	}
	else if(t->sharpright>sharpright_max)
	{
		config.sharpright = sharpright_max;
	}
	else
	{
		config.sharpright = t->sharpright; 
	}


	if(t->bumpup<bumpup_min)
	{
		config.bumpup = bumpup_min;
	}
	else if(t->bumpup>bumpup_max)
	{
		config.bumpup = bumpup_max;
	}
	else
	{
		config.bumpup = t->bumpup; 
	}


	if(t->bumpdown<bumpdown_min)
	{
		config.bumpdown = bumpdown_min;
	}
	else if(t->bumpdown>bumpdown_max)
	{
		config.bumpdown = bumpdown_max;
	}
	else
	{
		config.bumpdown = t->bumpdown; 
	}

	if(t->shake<shake_min)
	{
		config.shake = shake_min;
	}
	else if(t->shake>shake_max)
	{
		config.shake = shake_max;
	}
	else
	{
		config.shake = t->shake;
	}


	if(t->freq<freq_min)
	{
		config.freq = freq_min;
	}
	else if(t->freq>freq_max)
	{
		config.freq = freq_max;
	}
	else
	{
		config.freq = t->freq;
	}

	config.proc = t->proc;
	
	config.sharpsu *= conversion_rate;
	config.sharpsd *= conversion_rate;
	config.sharpbrake *= conversion_rate;
	config.sharpleft *= conversion_rate;
	config.sharpright *= conversion_rate;
	config.bumpup *=  conversion_rate;
	config.bumpdown *= conversion_rate;
	config.shake *= conversion_rate;

	pthread_create(&thProcess, NULL, process_shock, NULL);

	return 1;

}

void action_deinit()
{
	stop = 1;
	pthread_join(thProcess, NULL);
	close(fd6050);
}

const char* event[] = 
{
	"sharpspeedup_event",
	"sharpslowdown_event",
	"sharpbrake_event",
	"sharpturnleft_event",
	"sharpturnright_event",
	"bumpup_event",
	"bumpdown_event",
	"shake_event",
};

const char* get_event_desc(action_event evt)
{
	if(evt>shake_event)
	{
		return "invalid action event";
	}

	return event[evt];
}

static void report_event(action_event evt)
{
	printf("report event %s\r\n", get_event_desc(evt));
}

static u16 speed = 0;
u16 get_speed()
{
	return speed;
}


static void* process_shock(void* data)
{

	s16 currx;
	s16 curry;
	s16 currz;

	u32 t;
	u32 interval;


	data = data;

	interval = 1000000/config.freq;

    printf("interval is %d\r\n", interval);
	
	init_6050();

	while(!stop)
	{
		
		currx = (s16)get_data(ACCEL_XOUT_H);
		curry = (s16)get_data(ACCEL_YOUT_H);	
		currz = (s16)get_data(ACCEL_ZOUT_H);


		if(abs(currx)+abs(curry)+abs(currz)<shake_threshold)
		{
			state = wait_action;
			continue;
		}

		if(get_speed())
		{

			/*we have reported the event, so skip current state*/
			if(in_action == state)
			{
				continue;
			}
			
			if(currx>0 && (u32)currx>config.sharpsu)
			{
				state = in_action;
				config.proc(sharpspeedup_event);
			}

			if(currx<0 && abs(currx)>config.sharpsd && abs(currx)<=config.sharpbrake)
			{
				state = in_action;
				config.proc(sharpslowdown_event);
			}

			if(currx<0 && abs(currx)>config.sharpbrake)
			{
				state = in_action;
				config.proc(sharpbrake_event);
			}

			if(curry>0 && (u32)curry>config.sharpright)
			{
				state = in_action;
				config.proc(sharpturnright_event);
			}

			if(curry<0 && abs(curry)>config.sharpleft)
			{
				state = in_action;
				config.proc(sharpturnleft_event);
			}

			if(currz>0 && (u32)currz>(0X800+config.bumpdown))
			{
				state = in_action;
				config.proc(bumpdown_event);
			}

			if(currz<0 && (u32)(abs(currz)+0X800)>config.bumpup)
			{
				state = in_action;
				config.proc(bumpup_event);
			}

		}
		else
		{
			t = abs(currx)+abs(curry);

			if(currz>0)
			{
				t += abs(currz-0X800);
			}
			else
			{
				t = abs(currz);
			}

			if(t>config.shake)
			{
				state = in_action;
				config.proc(shake_event);
			}
			
		}
		
		usleep(interval);

	}

	return NULL;
}

int main(int argc, const char* argv[])
{
	char c;
	
	action_config cfg;

	bzero(&cfg, sizeof(cfg));
	
	cfg.proc = report_event;
	cfg.freq = 50;

	speed = atoi(argv[1]);
	
	action_init(&cfg);

	while(1)
	{
		c = getchar();
		if('q'==c)
		{
			action_deinit();
			break;
		}
	}	
	return 0;
}
#else
int action_init(const action_config* t) { return 0; }
void action_deinit() {}
#endif


