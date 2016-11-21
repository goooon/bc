#ifndef __BCP_VICP_SENDER_H__
#define __BCP_VICP_SENDER_H__

#include "../dep.h"
#include "../fundation.h"
#include "../util/Thread.h"
#include "../util/LinkedList.h"

#define VICP_SEND_OK      (0)
#define VICP_SEND_FAILED  (-1)
#define VICP_SEND_TIMEOUT (-2)
#define VICP_WAIT_ACK_TIMEOUT (-3)

typedef struct bcp_vicp_sender_s {
	mutex_type mutex; /* struct mutex */
	void *listener;
	int stop; /* send thread flag */
	int ack_stop; /* ack check timeout thread flag */
	List waiting_ack; /* packet for waiting ack */
	List waiting_send; /* packet for waiting sending */
	ListElement *send_head;
	sem_type sem; /* notify send thread */
	sem_type ack_sem; /* notify ack thread */
	mutex_type ack_mutex; /* ack list mutex */
	mutex_type send_mutex; /* send list mutex */
} bcp_vicp_sender_t;

/*
 * context, in param
 * result, packet send result, 0 = success
 */
typedef void (*vicp_sender_callback)(void *context, int result);

void bcp_vicp_sender_init(void);
void bcp_vicp_sender_uninit(void);

bcp_vicp_sender_t *bcp_vicp_sender_create(void *listener);

/* start sender sending thread */
int bcp_vicp_sender_start(bcp_vicp_sender_t *s);

/* send packet */
int bcp_vicp_sender_packet(bcp_vicp_sender_t *s,
	bcp_vicp_packet_t *p, int timeout, 
	vicp_sender_callback complete, void *context, u64 *id);

int bcp_vicp_send_data(bcp_vicp_sender_t *s,
	u8 *data, u16 len, int timeout, 
	vicp_sender_callback complete, void *context, u64 *id);

int bcp_vicp_send_ack(bcp_vicp_sender_t *s,
	u32 msg_id, u8 result, u16 code, int timeout,
	vicp_sender_callback complete, void *context, u64 *id);

void bcp_vicp_sender_notify_ack(bcp_vicp_sender_t *s, 
	u32 msg_id, int result);

/* stop sender sending thread */
int bcp_vicp_sender_stop(bcp_vicp_sender_t *s);
void bcp_vicp_sender_destroy(bcp_vicp_sender_t *s);

#endif // __BCP_VICP_SENDER_H__
