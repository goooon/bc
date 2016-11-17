#ifndef __BCP_VICP_RECEIVER_H__
#define __BCP_VICP_RECEIVER_H__

#include "../dep.h"
#include "../fundation.h"
#include "../util/Thread.h"
#include "../util/LinkedList.h"

typedef void (*fdata_arrived_callback)(void *context, 
	u8 *buf, u16 len);

typedef struct bcp_vicp_receiver_s {
	mutex_type mutex;
	void *listener;
	List received_list; /* packet for waiting post */
	mutex_type list_mutex;
	int stop; /* received thread */
	int post_stop; /* post thread */
	sem_type sem;
	fdata_arrived_callback dac;
	void *context;
	/*  for reading tag */
	u8 has_bytes;
	u8 last_bytes[4];
} bcp_vicp_receiver_t;

bcp_vicp_receiver_t *bcp_vcip_receiver_create(void *listener);
int bcp_vicp_receiver_start(bcp_vicp_receiver_t *r);

int bcp_vicp_receiver_data_arrived_callback(bcp_vicp_receiver_t *r, 
	fdata_arrived_callback dac, void *context);

int bcp_vicp_receiver_stop(bcp_vicp_receiver_t *r);
void bcp_vicp_receiver_destroy(bcp_vicp_receiver_t *r);

#endif // __BCP_VICP_RECEIVER_H__

