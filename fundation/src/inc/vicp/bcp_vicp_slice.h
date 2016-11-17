#ifndef __BCP_VICP_SLICE_H__
#define __BCP_VICP_SLICE_H__

#include "../dep.h"
#include "../fundation.h"
#include "../util/Thread.h"
#include "../util/LinkedList.h"

#define VICP_SLICE_SEND_OK 0
#define VICP_SLICE_SEND_FAILED -1
#define VICP_SLICE_SEND_TIMEOUT -2

typedef struct bcp_vicp_slicer_s {
	mutex_type mutex;
	mutex_type send_mutex;
	List sending;
	mutex_type recv_mutex;
	List received;
	u8 stop;
	sem_type sem;
	void *listener;
} bcp_vicp_slicer_t;

void bcp_vicp_slice_init(void);
void bcp_vicp_slice_uninit(void);

bcp_vicp_slicer_t *bcp_vicp_slice_create(void *listener);

int bcp_vicp_slice_start(bcp_vicp_slicer_t *slicer);
int bcp_vicp_slice_stop(bcp_vicp_slicer_t *slicer);

void bcp_vicp_slice_destroy(bcp_vicp_slicer_t *s);

int bcp_vicp_slice_regist_data_arrived_callback(
	bcp_vicp_receiver_t *r);

int bcp_vicp_slice_send(bcp_vicp_slicer_t *s,
	const char *buf, int len, int timeout, 
	vicp_sender_callback complete, void *context, u32 *id);

#endif // __BCP_VICP_SLICE_H__
