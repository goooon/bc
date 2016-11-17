#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "../../inc/dep.h"
#include "../../inc/util/Thread.h"
#include "../../inc/util/LinkedList.h"
#include "../../inc/util/Timestamp.h"

#include "../../inc/bcp_channel.h"

#include "../../inc/vicp/bcp_vicp_packet.h"
#include "../../inc/vicp/bcp_vicp_sender.h"
#include "../../inc/vicp/bcp_vicp.h"

#if !defined(_WIN32) && !defined(_WIN64)
#include <unistd.h>
#else
#include <windows.h>
#endif

#define MAX_RETRY_TIMES 5

typedef struct sender_pack_s {
	u64 id;
	bcp_vicp_packet_t *p;
	s64 escaped; /* timeout(ms) */
	int retry; /* retry times  */
	vicp_sender_callback complete;
	void *context;
} sender_pack_t;

static u64 seq_id = 0;
static mutex_type mutex = NULL;

void bcp_vicp_sender_init(void)
{
	mutex = Thread_create_mutex();
	if (!mutex) {
		LOG_W("bcp_vicp_sender_init create mutex failed.\n");
	}
}

void bcp_vicp_sender_uninit(void)
{
	Thread_destroy_mutex(mutex);
	mutex = NULL;
}

static u64 next_seq_id(void)
{
	u64 id;

	mutex_lock(mutex);
	id = seq_id++;	
	mutex_unlock(mutex);

	return id;
}

static void destroy_list(List *list)
{
	sender_pack_t *e;

	while ((e = (sender_pack_t*)ListDetachHead(list)) != NULL) {
		bcp_vicp_packet_destroy(e->p);
		free(e);
	}
}

static void destroy_ack_list(bcp_vicp_sender_t *s)
{
	mutex_lock(s->ack_mutex);
	destroy_list(&s->waiting_ack);
	mutex_unlock(s->ack_mutex);
}

static void destroy_sending_list(bcp_vicp_sender_t *s)
{
	mutex_lock(s->send_mutex);
	destroy_list(&s->waiting_send);
	mutex_lock(s->send_mutex);
}

static int packet_escaped(sender_pack_t *e)
{
	return (e && (current_timestamp() > e->escaped));
}

static int channel_write(bcp_channel_t *c, sender_pack_t *e, 
	const char *buf, int len)
{
	int ret, bytes = 0;
	int retry = 0;

	while (bytes != len) {
		ret = c->write(c, buf, len);
		if (ret < 0) {
			if (++retry < MAX_RETRY_TIMES) {
				msleep(10);
			} else {
				LOG_E("write dev failed. wrote=%d\n", bytes);
				return -1;
			}
		} else {
			bytes += ret;
		}
	}

	return 0;
}

static int send_one_packet(bcp_channel_t *c, sender_pack_t *e)
{
	int ret = 0;
	u8 *buf;
	u32 len;

	++e->retry;
	if (packet_escaped(e)) {
		return -1;
	}

	if (bcp_vicp_packet_serialize(e->p, &buf, &len) >= 0) {
		ret = channel_write(c, e, (const char*)buf, (int)len);
		free(buf);
	}

	return ret;
}

static void move_to_ack_list(bcp_vicp_sender_t *s, sender_pack_t *p)
{
	mutex_lock(s->ack_mutex);
	ListAppend(&s->waiting_ack, p, sizeof(*p));	
	mutex_unlock(s->ack_mutex);
	Thread_post_sem(s->ack_sem);
}

static void notify_complete(sender_pack_t *e, int result)
{
	if (e->complete) {
		(*e->complete)(e->context, result);
	}
	bcp_vicp_packet_destroy(e->p);
	free(e);
}

static void check_ack_list(bcp_vicp_sender_t *s)
{
	ListElement *current = NULL;
	sender_pack_t *e;

	mutex_lock(s->ack_mutex);

	while ((ListNextElement(&s->waiting_ack, &current)) != NULL) {
		e = (sender_pack_t*)current->content;
		if (packet_escaped(e)) {
			ListDetach(&s->waiting_ack, e);
			mutex_unlock(s->ack_mutex);
			notify_complete(e, VICP_SEND_TIMEOUT);
			mutex_lock(s->ack_mutex);
		}
	}

	mutex_unlock(s->ack_mutex);
}

/* ack packet coming */
void bcp_vicp_sender_notify_ack(bcp_vicp_sender_t *s, 
	u32 msg_id, int result)
{
	ListElement *current = NULL;
	sender_pack_t *e;

	if (!s) {
		return;
	}

	mutex_lock(s->ack_mutex);

	while (ListNextElement(&s->waiting_ack, &current) != NULL) {
		e = (sender_pack_t*)current->content;
		if (e && e->p && (e->p->msg_id == msg_id)) {
			ListDetach(&s->waiting_ack, e);
			mutex_unlock(s->ack_mutex);
			notify_complete(e, result);
			mutex_lock(s->ack_mutex);
			break;
		}
	}

	mutex_unlock(s->ack_mutex);
}

static void post_packet(bcp_channel_t *c, bcp_vicp_sender_t *s, sender_pack_t *e)
{
	int ret;

	if ((ret = send_one_packet(c, e)) >= 0) {
		if (e->p->type != VICP_PACKET_ACK) {
			move_to_ack_list(s, e);
		}
	} else {
		notify_complete(e, VICP_SEND_FAILED);
	}
}

static void send_packet(bcp_vicp_sender_t *s)
{
	bcp_channel_t *c;
	sender_pack_t *e;

	c = bcp_vicp_get_channel(s->listener);
	if (!c) {
		LOG_E("vicp sender request channel failed.\n");
		return;
	}

	mutex_lock(s->send_mutex);

	while ((e = (sender_pack_t*)ListDetachHead(&s->waiting_send)) != NULL) {
		mutex_unlock(s->send_mutex);
		post_packet(c, s, e);
		mutex_lock(s->send_mutex);
	}

	mutex_unlock(s->send_mutex);

	bcp_vicp_put_channel(s->listener);
}

static thread_return_type WINAPI sender_thread(void *arg)
{
	bcp_vicp_sender_t *s = (bcp_vicp_sender_t*)arg;

	if (!s) {
		return NULL;
	}

	mutex_lock(s->mutex);
	s->stop = 0;

	while (!s->stop) {
		mutex_unlock(s->mutex);
		Thread_wait_sem(s->sem, 1000);
		mutex_lock(s->mutex);
		send_packet(s);
	}

	destroy_sending_list(s);

	s->stop = 2; /* notify waiting thread */
	mutex_unlock(s->mutex);

	return NULL;
}

static thread_return_type WINAPI ack_thread(void *arg)
{
	bcp_vicp_sender_t *s = (bcp_vicp_sender_t*)arg;

	if (!s) {
		return NULL;
	}

	mutex_lock(s->mutex);
	s->ack_stop = 0;

	while (!s->ack_stop) {
		mutex_unlock(s->mutex);
		Thread_wait_sem(s->ack_sem, 500);
		mutex_lock(s->mutex);
		check_ack_list(s);
	}

	destroy_ack_list(s);

	s->ack_stop = 2; /* notify waiting thread */
	mutex_unlock(s->mutex);

	return NULL;
}

bcp_vicp_sender_t *bcp_vicp_sender_create(void *listener)
{
	bcp_vicp_sender_t *s;

	s = (bcp_vicp_sender_t*)malloc(sizeof(*s));
	if (!s) {
		return NULL;
	}

	ListZero(&s->waiting_send);
	ListZero(&s->waiting_ack);

	s->mutex = Thread_create_mutex();
	if (!s->mutex) {
		goto __failed;
	}
	s->ack_mutex = Thread_create_mutex();
	if (!s->ack_mutex) {
		goto __failed;
	}
	s->send_mutex = Thread_create_mutex();
	if (!s->send_mutex) {
		goto __failed;
	}
	s->sem = Thread_create_sem();
	if (!s->sem) {
		goto __failed;
	}
	s->ack_sem = Thread_create_sem();
	if (!s->ack_sem) {
		goto __failed;
	}

	s->stop = 1;
	s->listener = listener;
	bcp_vicp_get_listener(listener);

	return s;

__failed:
	if (s) {
		if (s->mutex)
			Thread_destroy_mutex(s->mutex);
		if (s->ack_mutex)
			Thread_destroy_mutex(s->ack_mutex);
		if (s->send_mutex)
			Thread_destroy_mutex(s->send_mutex);
		if (s->sem)
			Thread_destroy_sem(s->sem);
		if (s->ack_sem)
			Thread_destroy_sem(s->ack_sem);
		free(s);
	}
	return NULL;
}


static int start_thread(bcp_vicp_sender_t *s, 
	int *stop, thread_fn fn)
{
	mutex_lock(s->mutex);

	if (!*stop) {
		LOG_W("vicp sending thread has been started\n");
		mutex_unlock(s->mutex);
		return -1;
	}

	Thread_start(fn, s);

	/* waiting already started */
	while (*stop != 0) {
		mutex_unlock(s->mutex);
		msleep(100);
		mutex_lock(s->mutex);
	}

	mutex_unlock(s->mutex);
	return 0;
}

int bcp_vicp_sender_start(bcp_vicp_sender_t *s)
{
	if (!s) {
		return -1;
	}

	start_thread(s, &s->stop, sender_thread);
	start_thread(s, &s->ack_stop, ack_thread);

	return 0;
}

int bcp_vicp_sender_packet(bcp_vicp_sender_t *s,
	bcp_vicp_packet_t *p, int timeout, 
	vicp_sender_callback complete, void *context, u64 *id)
{
	sender_pack_t *sp;

	if (!s || !p) {
		return -1;
	}

	sp = (sender_pack_t*)malloc(sizeof(*sp));
	if (!sp) {
		return -1;
	}

	memset(sp, 0, sizeof(*sp));
	sp->id = next_seq_id();
	sp->p = p;
	sp->escaped = current_timestamp() + timeout;
	sp->retry = 0;
	sp->complete = complete;
	sp->context = context;

	mutex_lock(s->mutex);
	ListAppend(&s->waiting_send, sp, sizeof(*sp));
	mutex_unlock(s->mutex);

	/* wakeup sender thread */
	Thread_post_sem(s->sem);

	if (id) {
		*id = sp->id;
	}

	return 0;
}

int bcp_vicp_send_data(bcp_vicp_sender_t *s,
	u8 *data, u16 len, int timeout, 
	vicp_sender_callback complete, void *context, u64 *id)
{
	bcp_vicp_packet_t *vp;

	vp = bcp_vicp_create_request(bcp_vicp_next_seq_id(), 
		data, len);
	if (!vp) {
		return -1;
	}

	if (bcp_vicp_sender_packet(s,
		vp, timeout, complete, context, id) < 0) {
		bcp_vicp_packet_destroy(vp);
		return -1;
	}
	return 0;
}

int bcp_vicp_send_ack(bcp_vicp_sender_t *s,
	u8 result, u16 code, int timeout, 
	vicp_sender_callback complete, void *context, u64 *id)
{
	bcp_vicp_packet_t *vp;

	vp = bcp_vicp_create_ack(bcp_vicp_next_seq_id(), 
		result, code);
	if (!vp) {
		return -1;
	}

	if (bcp_vicp_sender_packet(s,
		vp, timeout, complete, context, id) < 0) {
		bcp_vicp_packet_destroy(vp);
		return -1;
	}
	return 0;
}


static int stop_thread(bcp_vicp_sender_t *s,
	int *stop)
{

	mutex_lock(s->mutex);

	if (*stop) {
		LOG_W("vicp sending thread has stoped\n");
		mutex_unlock(s->mutex);
		return -1;
	}

	*stop = 1;

	/* waiting already stoped */
	while (*stop != 2) {
		mutex_unlock(s->mutex);
		msleep(100);
		mutex_lock(s->mutex);
	}

	mutex_unlock(s->mutex);
	return 0;
}

int bcp_vicp_sender_stop(bcp_vicp_sender_t *s)
{
	if (!s) {
		return -1;
	}

	stop_thread(s, &s->stop);
	stop_thread(s, &s->ack_stop);

	return 0;
}

void bcp_vicp_sender_destroy(bcp_vicp_sender_t *s)
{
	bcp_vicp_sender_stop(s);
	bcp_vicp_put_listener(s->listener);

	Thread_destroy_sem(s->sem);
	Thread_destroy_sem(s->ack_sem);
	Thread_destroy_mutex(s->mutex);
	Thread_destroy_mutex(s->ack_mutex);
	Thread_destroy_mutex(s->send_mutex);
	free(s);
}
