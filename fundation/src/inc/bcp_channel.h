#ifndef __BCP_CHANNEL_H__
#define __BCP_CHANNEL_H__

#include "../inc/dep.h"
#include "../inc/util/Thread.h"

/*
 * abstrace channel of data stream base on serial, usb... 
 */

#define BCP_CHANNEL_SERIAL 1
#define BCP_CHANNEL_USB 2
#define BCP_CHANNEL_BLUETOOTH 3
#define BCP_CHANNEL_NDEVICE 4 /* base on ndevice */

typedef struct bcp_channel_s {
	int type;
	char *dev_name; /* such as: /dev/ttySAC0 */
	void *hdl;
	int hdl_ref;
	int channel_ref;
	mutex_type mutex;
	int packet_size; /* max size of per packet for once trans on channel */
	int (*open)(void *channel);
	int (*read)(void *channel, char *buf, int len, int timeout/*ms*/);
	int (*write)(void *channel, const char *buf, int len);
	int (*close)(void *channel);
	void *listener; /* vicp listener */
} bcp_channel_t;

void bcp_channel_init(void);
void bcp_channel_uninit(void);

bcp_channel_t *bcp_channel_create(int type, const char *dev_name);
void bcp_channel_destroy(bcp_channel_t *c);

bcp_channel_t *bcp_channel_get(bcp_channel_t *c);
bcp_channel_t *bcp_channel_get_byname(int type, const char *dev_name);
void bcp_channel_put(bcp_channel_t *c);

#endif // __BCP_CHANNEL_H__
