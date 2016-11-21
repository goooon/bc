#ifndef __BCP_VICP_H__
#define __BCP_VICP_H__

#include "../dep.h"
#include "../fundation.h"
#include "../util/Thread.h"
#include "../util/LinkedList.h"
#include "../bcp_channel.h"
#include "./bcp_vicp_packet.h"
#include "./bcp_vicp_receiver.h"
#include "./bcp_vicp_sender.h"
#include "./bcp_vicp_slice.h"

#define mutex_lock(m) Thread_lock_mutex(m)
#define mutex_unlock(m) Thread_unlock_mutex(m)

/*
 * print detail VICP
 */
//#define PRINT_VICP_DETAIL 1

/*
 * mock receiver, sender from local list
 */
//#define VICP_MOCK_TEST 1

/*
 * vechicle Inter-Connect Protocol
 * functions:
 *     1. codec data by vicp
 *     2. listening all channel by registered
 *     3. read package from channel
 *     4. write package to channel
 *     5. nofiy up package arrivied
 */

typedef void (*vicp_data_arrived_callback)(void *context, u8 *buf, u16 len);

typedef struct vicp_listener_s {
	bcp_channel_t *ch;
	bcp_vicp_sender_t *sender;
	bcp_vicp_receiver_t *receiver;
	bcp_vicp_slicer_t *slicer;
	int ref;
	mutex_type mutex;
	vicp_data_arrived_callback vdac;
	void *context;
} vicp_listener_t;

void bcp_vicp_init(void);
void bcp_vicp_uninit(void);

int bcp_vicp_regist_channel(bcp_channel_t *c);
int bcp_vicp_unregist_channel(bcp_channel_t *c);

int bcp_vicp_send(bcp_channel_t *c,
	const char *buf, int len, vicp_sender_callback complete, 
	void *context, u32 *id);

int bcp_vicp_regist_data_arrived_callback(
	bcp_channel_t *c,
	vicp_data_arrived_callback cb,
	void *context);

void bcp_vicp_get_listener(void *listener);
void bcp_vicp_put_listener(void *listener);

bcp_channel_t *bcp_vicp_get_channel(void *listener);

/* for inner function */
void bcp_vicp_data_arrived(void *listener, 
	u8 *buf, u16 len);

#endif // __BCP_VICP_H__
