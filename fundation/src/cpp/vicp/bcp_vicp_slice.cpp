#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "../../inc/dep.h"
#include "../../inc/util/Thread.h"
#include "../../inc/util/LinkedList.h"
#include "../../inc/util/Timestamp.h"
#include "../../inc/binary_formater.h"

#include "../../inc/bcp_channel.h"

#include "../../inc/vicp/bcp_vicp_packet.h"
#include "../../inc/vicp/bcp_vicp_receiver.h"
#include "../../inc/vicp/bcp_vicp_sender.h"
#include "../../inc/vicp/bcp_vicp_slice.h"
#include "../../inc/vicp/slice_proto.h"
#include "../../inc/vicp/bcp_vicp.h"

#define DEF_WAIT_ACK_TIMEOUT (10 * 1000)
#define MAX_RECV_SLICE_TIMEOUT (40 * 1000)

/*
 * slice context state
 */
#define SLICE_WAIT_GROUP_ID 0
#define SLICE_WAIT_DESC		1
#define SLICE_WAIT_DESC_ACK 2
#define SLICE_WAIT_DATA 	3
#define SLICE_WAIT_DATA_ACK 4

#define SLICE_RECV 1
#define SLICE_SEND 2

/*
 * slice context for send/recv
 */
typedef struct slice_context_s {
	mutex_type mutex;
	u64 seq_id;
	u8 direction;
	u8 state;
	bcp_vicp_slicer_t *slicer;
	u32 context_id;
	u32 group_id;
	u16 slice_id;
	u8 *data;
	u16 len;
	u16 index; /* next send/receive data location */
	int timeout; /* send/receive data timeout(ms) */
	s64 timestamp;
	u16 slice_count;
	u16 per_slice_size;
	u16 trans_len; /* recv/send trans length */
	vicp_sender_callback complete;
	void *context;
	int ref;
} slice_context_t;

typedef struct slice_callback_s {
	bcp_vicp_slicer_t *slicer;
	slice_context_t *sc;
} slice_callback_t;

static u32 context_id = 0;
static u32 group_id = 0;
static mutex_type mutex = NULL;

static void destroy_slice_list(List *list, mutex_type mutex);

void bcp_vicp_slice_init(void)
{
	mutex = Thread_create_mutex();
	if (!mutex) {
		LOG_W("bcp_vicp_slice_init create mutex failed.\n");
		return;
	}
}

void bcp_vicp_slice_uninit(void)
{
	if (mutex) {
		Thread_destroy_mutex(mutex);
		mutex = NULL;
	}
}

static u32 next_context_id(void)
{
	u32 id;

	mutex_lock(mutex);
	id = context_id++;	
	mutex_unlock(mutex);
	return id;
}

static u32 next_group_id(void)
{
	u32 id;

	mutex_lock(mutex);
	id = group_id++;	
	mutex_unlock(mutex);
	return id;
}

/* sender find context by context id */
static int find_cb(void *context, void *context_id)
{
	slice_context_t *c = (slice_context_t*)context;

	return (c && (c->context_id == *(u32*)context_id));
}

static slice_context_t *find_slice_context(List *list, u32 context_id)
{
	ListElement *e;

	e = ListFindItem(list, &context_id, find_cb);
	if (e) {
		return (slice_context_t*)e->content;
	} else {
		return NULL;
	}
}

/* receiver find context by group id */
static int find_cb_by_groupid(void *context, void *group_id)
{
	slice_context_t *c = (slice_context_t*)context;

	return (c && (c->group_id == *(u32*)group_id));
}

static slice_context_t *find_slice_context_by_groupid(List *list, 
	u32 group_id)
{
	ListElement *e;

	e = ListFindItem(list, &group_id, find_cb_by_groupid);
	if (e) {
		return (slice_context_t*)e->content;
	} else {
		return NULL;
	}
}

static void destroy_slice_context(slice_context_t *sc)
{
	if (sc) {
		Thread_destroy_mutex(sc->mutex);
		if (sc->data) {
			free(sc->data);
		}
		free(sc);
	}
}

static void destroy_slice_list(List *list, mutex_type mutex)
{
	slice_context_t *sc;

	mutex_lock(mutex);
	while ((sc = (slice_context_t*)ListDetachHead(list)) != NULL) {
		destroy_slice_context(sc);
	}
	mutex_unlock(mutex);
}

static void wait_context_unused(slice_context_t *sc)
{
	mutex_lock(sc->mutex);
	while (sc->ref != 0) {
		LOG_E("slice context %p having used.\n", sc);
		mutex_unlock(sc->mutex);
		msleep(100);
		mutex_lock(sc->mutex);
	}
	mutex_unlock(sc->mutex);
}

static void add_ref(slice_context_t *sc)
{
	if (sc) {
		mutex_lock(sc->mutex);
		sc->ref++;
		mutex_unlock(sc->mutex);
	}
}

static void dec_ref(slice_context_t *sc)
{
	if (sc) {
		mutex_lock(sc->mutex);
		if (sc->ref == 0) {
			LOG_E("slice context[%p] ref == 0.\n", sc);
			mutex_unlock(sc->mutex);
			return;
		}
		sc->ref--;
		mutex_unlock(sc->mutex);
	}
}

static void free_and_notify_context(slice_context_t *sc, 
	int result)
{
	wait_context_unused(sc);

	if (sc->complete) {
		(*sc->complete)(sc->context, result);
	}

	destroy_slice_context(sc);
}

static void free_send_context_p(bcp_vicp_slicer_t *s, 
	slice_context_t *sc, int result)
{
	mutex_lock(s->send_mutex);
	if (!ListDetach(&s->sending, sc)) {
		mutex_unlock(s->send_mutex);
		return;
	}
	mutex_unlock(s->send_mutex);
	free_and_notify_context(sc, result);
}

static void free_send_context(slice_context_t *sc,
	int result)
{
	free_send_context_p(sc->slicer, sc, result);
}

static void free_recv_context(slice_context_t *sc)
{
	bcp_vicp_slicer_t *s = sc->slicer;

	mutex_lock(s->recv_mutex);
	if (!ListDetach(&s->received, sc)) {
		mutex_unlock(s->recv_mutex);
		return;
	}
	mutex_unlock(s->recv_mutex);

	wait_context_unused(sc);
	destroy_slice_context(sc);
}

/*
 * sender request ack callback
 */
static void slice_send_callback(void *context, 
	int result)
{
	slice_callback_t *cb = (slice_callback_t*)context;
	slice_context_t *sc;
	bcp_vicp_slicer_t *slicer;

	if (!cb) {
		return;
	}

	sc = cb->sc;
	slicer = cb->slicer;

	if (result != 0) {
		mutex_lock(slicer->send_mutex);
		if (!ListDetach(&slicer->sending, sc)) {
			sc = NULL;
		}
		mutex_unlock(slicer->send_mutex);
		if (sc) {
			if (sc->state == SLICE_WAIT_GROUP_ID) {
				LOG_E("request group id failed, result = %d.\n", result);
			} else if (sc->state == SLICE_WAIT_DESC_ACK) {
				LOG_E("slice desc send failed, result = %d.\n", result);
			} else if (sc->state == SLICE_WAIT_DATA_ACK) {
				LOG_E("slice data(%d,%d) send failed, result = %d.\n", 
					sc->len, sc->index, result);
			} else {
				LOG_E("slice invalid state, result = %d.\n", result);
			}
			free_and_notify_context(sc, VICP_SLICE_SEND_FAILED);
		} else {
			LOG_E("slice send callback, result = %d.\n", result);
		}
	}

	free(cb);
}

/*
 * receiver response ack callback
 */
static void slice_recv_callback(void *context, int result)
{
	slice_callback_t *cb = (slice_callback_t*)context;
	slice_context_t *sc;
	bcp_vicp_slicer_t *slicer;

	if (!cb) {
		return;
	}

	sc = cb->sc;
	slicer = cb->slicer;

	if (result < 0) {
		if (!sc) {
			LOG_E("slice_recv_callback result = %d.\n", result);
		} else {
			LOG_E("slice_recv_callback [%p] result = %d.\n", sc, result);
		}
	}

	free(cb);
}

static bcp_vicp_sender_t *get_sender(bcp_vicp_slicer_t *s)
{
	vicp_listener_t *l;
	l = (vicp_listener_t*)s->listener;
	return l->sender;
}

static int send_stream(bcp_vicp_slicer_t *slicer, 
	slice_context_t *sc, 
	bf_t *f, vicp_sender_callback complete)
{
	u8 *stream;
	u16 size;
	u64 *id = NULL;
	int timeout = DEF_WAIT_ACK_TIMEOUT;
	slice_callback_t *context;

	stream = bf_stream(f);
	size = bf_size(f);
	if (sc) {
		id = &sc->seq_id;
		timeout = sc->timeout;
	}

	if (complete) {
		context = (slice_callback_t*)malloc(sizeof(*context));
		if (context) {
			context->slicer = slicer;
			context->sc = sc;
		}
	} else {
		context = NULL;
	}

	return bcp_vicp_send_data(get_sender(slicer), stream, size, 
		timeout, complete, context, id);
}

static int request_group_id(slice_context_t *sc)
{
	int ret;
	bf_t f;

	if (create_group_id_req(&f, sc->context_id) < 0) {
		return -1;
	}

	mutex_lock(sc->mutex);
	sc->state = SLICE_WAIT_GROUP_ID;
	mutex_unlock(sc->mutex);

	ret = send_stream(sc->slicer, sc, &f, slice_send_callback);
	free_slice_req(&f);

	return ret;
}

static u16 channel_packet_size(bcp_vicp_sender_t *sender)
{
	u16 size;
	bcp_channel_t *c;

	if (!(c = bcp_vicp_get_channel(sender->listener))) {
		LOG_E("channel_packet_size failed.\n");
		return 0;
	}

	size = c->packet_size;
	return size;
}

static u16 slice_count(slice_context_t *sc)
{
	u16 count;

	count = sc->len / sc->per_slice_size;
	if ((sc->len % sc->per_slice_size) > 0) {
		count++;
	}

	return count;
}

static int respone_group_id(bcp_vicp_slicer_t *s, bf_t *bf)
{
	int ret;
	bf_t f;
	slice_group_t g;

	if (parse_group_id_req(bf, &g)) {
		return -1;
	}

	if (create_group_id_ack(&f, g.context_id, 
		next_group_id(), channel_packet_size(get_sender(s))) < 0) {
		return -1;
	}

	ret = send_stream(s, NULL, &f, slice_recv_callback);
	free_slice_req(&f);

	return ret;
}

static int send_slice_desc(bcp_vicp_slicer_t *s, bf_t *bf)
{
	int ret = -1;
	bf_t f;
	slice_context_t *sc;
	slice_group_ack_t g;
	u8 result = 0;

	if (parse_group_id_ack(bf, &g) < 0) {
		return -1;
	}

	mutex_lock(s->send_mutex);
	sc = find_slice_context(&s->sending, g.context_id);
	if (!sc) {
		mutex_unlock(s->send_mutex);
		return -1;
	}
	add_ref(sc);
	mutex_unlock(s->send_mutex);

	mutex_lock(sc->mutex);

	/* sender get group id */
	sc->group_id = g.group_id;
	sc->per_slice_size = g.slice_size;
	sc->slice_count = slice_count(sc);
	sc->timestamp += sc->timeout;

	//LOG_I("send[start] context_id=%d, group_id=%d, len=%d, slice_count=%d\n",
	//	sc->context_id, sc->group_id, sc->len, sc->slice_count);

	if (create_desc_req(&f, g.context_id, 
		g.group_id, sc->len, sc->slice_count) < 0) {
		goto __failed;
	}

	sc->state = SLICE_WAIT_DESC_ACK;

	if (send_stream(s, sc, &f, slice_send_callback) < 0) {
		goto __failed;
	}

	ret = 0;

__failed:
	mutex_unlock(sc->mutex);
	dec_ref(sc);
	free_slice_req(&f);

	return ret;
}

/* receiver create slice context */
static slice_context_t *create_recv_context(
	bcp_vicp_slicer_t *s, slice_desc_t *desc)
{
	slice_context_t *sc;

	sc = (slice_context_t*)malloc(sizeof(*sc));
	if (!sc) {
		return NULL;
	}

	sc->mutex = Thread_create_mutex();
	if (!sc->mutex) {
		free(sc);
		return NULL;
	}

	sc->slicer = s;
	sc->group_id = desc->group_id;
	sc->slice_id = 0;
	sc->data = (u8*)malloc(desc->len);
	if (!sc->data) {
		free(sc);
		Thread_destroy_mutex(sc->mutex);
		return NULL;
	}
	sc->len = desc->len;
	sc->timeout = DEF_WAIT_ACK_TIMEOUT;

	/* for sender only */
	sc->index = 0;
	sc->complete = NULL;
	sc->context = NULL;

	sc->state = SLICE_WAIT_DATA;
	sc->timestamp = current_timestamp() + MAX_RECV_SLICE_TIMEOUT;
	sc->context_id = next_context_id();
	sc->per_slice_size = channel_packet_size(get_sender(s));
	sc->slice_count = desc->slice_count;
	sc->trans_len = 0;
	sc->ref = 0;
	sc->direction = SLICE_RECV;

	ListAppend(&s->received, sc, sizeof(*sc));

	return sc;
}

static int response_slice_desc_ack(bcp_vicp_slicer_t *s, bf_t *bf)
{
	int ret;
	bf_t f;
	slice_context_t *sc;
	slice_desc_t desc;
	u8 result = 0;

	if (parse_desc_req(bf, &desc) < 0) {
		return -1;
	}

	//LOG_I("recv[start] context_id=%d, group_id=%d, len=%d, slice_count=%d\n", 
	//	desc.context_id, desc.group_id, desc.len, desc.slice_count);

	/* receiver, find slice_context, create if not found */
	mutex_lock(s->recv_mutex);
	sc = find_slice_context_by_groupid(&s->received, desc.group_id);
	if (!sc) {
		sc = create_recv_context(s, &desc);
		if (!sc) {
			LOG_E("create slice failed. group_id=%d\n", desc.group_id);
			result = 1;
		}
	}
	add_ref(sc);
	mutex_unlock(s->recv_mutex);

	if (sc) {
		mutex_lock(sc->mutex);
		sc->state = SLICE_WAIT_DATA;
		mutex_unlock(sc->mutex);
	}

	if (create_desc_ack(&f, desc.context_id, 
		desc.group_id, result) < 0) {
		goto __failed;
	}

	if (send_stream(s, sc, &f, slice_recv_callback) < 0) {
		goto __failed;
	}

	ret = 0;

__failed:
	dec_ref(sc);
	free_slice_req(&f);
	return ret;
}

static int put_data(slice_context_t *sc, slice_data_t *data)
{
	u32 index;

	index = data->slice_id * sc->per_slice_size;

	if (data->slice_id >= sc->slice_count) {
		LOG_E("recv slice id exception [%p] slice_id=%d, slice_count=%d\n",
			sc, data->slice_id, sc->slice_count);
		return -1;
	} else if ((sc->trans_len + data->len) > sc->len) {
		LOG_E("recv data overflow [%p] slice_id=%d, data_len=%d, total_size=%d\n",
			sc, data->slice_id, data->len, sc->len);
		return -1;
	}

	memcpy(&sc->data[index], data->buf, data->len);
	sc->trans_len += data->len;

	return 0;
}

static int trans_complete(slice_context_t *sc)
{
	return (sc->trans_len == sc->len);
}

static int notify_data_arrived(slice_context_t *sc)
{
	u8 *buf;

	buf = (u8*)malloc(sc->len);
	if (!buf) {
		LOG_E("notify_data_arrived malloc failed.\n");
		return -1;
	} else {
		memcpy(buf, sc->data, sc->len);
	}

	bcp_vicp_data_arrived(sc->slicer->listener, buf, sc->len);

	return 0;
}

static int recv_slice(bcp_vicp_slicer_t *s, bf_t *bf)
{
	bf_t f;
	slice_context_t *sc;
	slice_data_t data;

	int ret;
	u8 recv_complete, result, code;

	ret = -1;
	recv_complete = 0;
	result = 0;
	code = 0;

	if (parse_data_req(bf, &data) < 0) {
		return -1;
	}

	/* receiver, find slice_context */
	mutex_lock(s->recv_mutex);
	sc = find_slice_context_by_groupid(&s->received, data.group_id);
	if (!sc) {
		LOG_E("recv can not found slice context group_id = %d\n",
			data.group_id);
		mutex_unlock(s->recv_mutex);
		return -1;
	}

	add_ref(sc);
	mutex_unlock(s->recv_mutex);

	mutex_lock(sc->mutex);
	sc->state = SLICE_WAIT_DATA;
	sc->timestamp += MAX_RECV_SLICE_TIMEOUT;

	//LOG_I("recv context_id=%d, group_id=%d, slice_id=%d, len=%d\n", 
	//	data.context_id, data.group_id, data.slice_id, data.len);

	/* put data to slice context */
	if (put_data(sc, &data) < 0) {
		result = 1;
	} else {
		if (trans_complete(sc)) {
			notify_data_arrived(sc);
			recv_complete = 1;
		} else {
			/* wake up thread */
			Thread_post_sem(s->sem);
		}
	}

	if (create_data_ack(&f, data.context_id, 
		data.group_id, data.slice_id, data.len, result, code) < 0) {
		goto __failed;
	}

	if (send_stream(s, sc, &f, slice_recv_callback) < 0) {
		goto __failed;
	}

	ret = 0;

__failed:
	mutex_unlock(sc->mutex);
	dec_ref(sc);
	if (recv_complete) {
		free_recv_context(sc);
	}
	free_slice_req(&f);

	return ret;	
}

static void notify_data_delivered(slice_context_t *sc, u8 result)
{
	if (sc->complete) {
		(*sc->complete)(sc->context, result);
	}
}

int send_next_slice(bcp_vicp_slicer_t *s, u8 type, 
	bf_t *bf)
{
	int ret = -1;
	bf_t f;
	slice_context_t *sc;
	slice_desc_ack_t dack;
	slice_data_ack_t ack;

	u32 context_id, group_id;
	u8 result;

	u8 *data;
	u16 len, ack_len, slice_id;

	if (type == VICP_SLICE_DESC_ACK) {
		if (parse_desc_ack(bf, &dack) < 0) {
			return -1;
		}
		context_id = dack.context_id;
		group_id = dack.group_id;
		result = dack.result;
		ack_len = 0;
	} else if (type == VICP_SLICE_DATA_ACK) {
		if (parse_data_ack(bf, &ack) < 0) {
			return -1;
		}
		context_id = ack.context_id;
		group_id = ack.group_id;
		result = ack.result;
		ack_len = ack.len;
	} else {
		return -1;
	}

	/* sender, find slice_context */
	mutex_lock(s->send_mutex);
	sc = find_slice_context(&s->sending, context_id);
	if (!sc) {
		LOG_E("sending can not find slice context_id = %d\n",
			context_id);
		mutex_unlock(s->send_mutex);
		return -1;
	} else if (result != 0) {
		LOG_E("sending data ack != 0 context_id = %d, result=%d.\n",
			context_id, result);
		mutex_unlock(s->send_mutex);
		free_send_context(sc, VICP_SLICE_SEND_FAILED);
		return -1;
	}

	add_ref(sc);
	mutex_unlock(s->send_mutex);

	mutex_lock(sc->mutex);
	sc->trans_len += ack_len;

	if (trans_complete(sc)) {
		mutex_unlock(sc->mutex);
		dec_ref(sc);
		free_send_context(sc, VICP_SLICE_SEND_OK);
		return 0;
	} else if (sc->slice_id >= sc->slice_count) {
		LOG_E("sending trans failed (slice_id = %d) >= (slice_count = %d).\n",
			sc->slice_id, sc->slice_count);
		mutex_unlock(s->send_mutex);
		dec_ref(sc);
		free_send_context(sc, VICP_SLICE_SEND_FAILED);
		return -1;
	}

	slice_id = sc->slice_id++;
	sc->timestamp += sc->timeout;

	data = &sc->data[sc->index];
	sc->index += sc->per_slice_size;
	if (sc->index <= sc->len) {
		len = sc->per_slice_size;
	} else {
		len = sc->len % sc->per_slice_size;
	}

	sc->state = SLICE_WAIT_DATA_ACK;

	//LOG_I("send context_id=%d, group_id=%d, slice_id=%d, len=%d\n",
	//	sc->context_id, sc->group_id, slice_id, len);

	if (create_data_req(&f, context_id, 
		group_id, slice_id, data, len) < 0) {
		goto __failed;
	}

	if (send_stream(s, sc, &f, slice_send_callback) < 0) {
		goto __failed;
	}

	ret = 0;
	
__failed:
	mutex_unlock(sc->mutex);
	dec_ref(sc);
	free_slice_req(&f);

	return ret;
}

int bcp_vicp_slice_send(bcp_vicp_slicer_t *s,
	const char *buf, int len, int timeout, 
	vicp_sender_callback complete, void *context, u32 *id)
{
	slice_context_t *sc;

	if (!s || !buf) {
		return -1;
	}

	sc = (slice_context_t*)malloc(sizeof(*sc));
	if (!sc) {
		return -1;
	}

	memset(sc, 0, sizeof(*sc));

	sc->mutex = Thread_create_mutex();
	if (!sc->mutex) {
		free(sc);
		return -1;
	}
	sc->slicer = s;
	sc->group_id = 0;
	sc->slice_id = 0;
	sc->data = (u8*)malloc(len);
	if (!sc->data) {
		free(sc);
		return -1;
	} else {
		memcpy(sc->data, buf, len);
	}
	sc->len = (u16)len;
	sc->index = 0;
	sc->timeout = timeout;
	sc->complete = complete;
	sc->context = context;
	sc->direction = SLICE_SEND;

	sc->state = SLICE_WAIT_GROUP_ID;
	sc->timestamp = current_timestamp() + sc->timeout;
	sc->context_id = next_context_id();

	sc->slice_count = 0;
	sc->ref = 0;
	sc->trans_len = 0;

	if (request_group_id(sc) < 0) {
		destroy_slice_context(sc);
		return -1;
	}

	mutex_lock(s->send_mutex);
	ListAppend(&s->sending, sc, sizeof(*sc));
	mutex_unlock(s->send_mutex);

	if (id) {
		*id = sc->context_id;
	}

	return 0;
}

static int slice_escaped(slice_context_t *sc)
{
	s64 current = current_timestamp();
	return (sc && sc->timestamp < current);
}

static void recvd_slice_escaped(bcp_vicp_slicer_t *s)
{
	ListElement *current = NULL;
	slice_context_t *sc;
	
	mutex_lock(s->recv_mutex);

	/* check list timeout */
	while ((ListNextElement(&s->received, &current)) != NULL) {
		sc = (slice_context_t*)current->content;
		if (slice_escaped(sc)) {
			LOG_E("slice received timeout, group_id=%d\n", 
				sc->group_id);
			mutex_unlock(s->recv_mutex);
			free_recv_context(sc);
			mutex_lock(s->recv_mutex);
			current = NULL; /* TODO: */
		}
	}

	mutex_unlock(s->recv_mutex);
}

static void send_slice_escaped(bcp_vicp_slicer_t *s)
{
	ListElement *current = NULL;
	slice_context_t *sc;
	
	mutex_lock(s->send_mutex);

	/* check list timeout */
	while ((ListNextElement(&s->sending, &current)) != NULL) {
		sc = (slice_context_t*)current->content;
		if (slice_escaped(sc)) {
			LOG_E("slice send timeout, context_id=%d\n", 
				sc->context_id);
			mutex_unlock(s->send_mutex);
			free_send_context(sc, VICP_SLICE_SEND_TIMEOUT);
			mutex_lock(s->send_mutex);
			current = NULL; /* TODO: */
		}
	}

	mutex_unlock(s->send_mutex);
}

static thread_return_type WINAPI slice_thread(void *arg)
{
	bcp_vicp_slicer_t *s = (bcp_vicp_slicer_t*)arg;

	if (!s) {
		return NULL;
	}

	mutex_lock(s->mutex);
	s->stop = 0;

	while (!s->stop) {
		mutex_unlock(s->mutex);
		Thread_wait_sem(s->sem, 500);
		recvd_slice_escaped(s);
		//send_slice_escaped(s);
		mutex_lock(s->mutex);
	}

	s->stop = 2;
	mutex_unlock(s->mutex);

	return NULL;
}

static int proto_valid(bf_t *f, u8 *t)
{
	u8 r, type, ver;

	if (bf_read_u8(f, &r) < 0) {
		return 0;
	}

	type = (r >> 4) & 0xf;
	ver = r & 0xf;

	if (ver != VICP_SLICE_VERSION) {
		LOG_E("slice arrived, slice version invalid, ver=%d\n", ver);
		return 0;
	}

	*t = type;

	return 1;
}

static int bcp_vicp_slice_arrived(bcp_vicp_slicer_t *slicer, 
	u8 *data, u16 len)
{
	int ret = -1;
	u8 type;
	bf_t f;

	if (!data) {
		return -1;
	}

	bf_init_d(&f, data, len);

	if (!proto_valid(&f, &type)) {
		free(data);
		return -1;
	}

	bf_reset(&f, 0);

	if (type == VICP_SLICE_GROUP_ID) {
		ret = respone_group_id(slicer, &f);
	} else if (type == VICP_SLICE_GROUP_ID_ACK) {
		ret = send_slice_desc(slicer, &f);
	} else if (type == VICP_SLICE_DESC) {
		ret = response_slice_desc_ack(slicer, &f);
	} else if (type == VICP_SLICE_DATA) {
		ret = recv_slice(slicer, &f);
	} else if (type == VICP_SLICE_DESC_ACK 
		|| type == VICP_SLICE_DATA_ACK) {
		ret = send_next_slice(slicer, type, &f);
	} else if (type == VICP_SLICE_ONLY_ONE) {
		/* TODO: */
		ret = -1;
	} else {
		LOG_E("invalude slice proto, type = %d\n", type);
	}

	free(data);

	return ret;
}

static void data_arrived_callback(void *context, u8 *buf, u16 len)
{
	vicp_listener_t *l = (vicp_listener_t*)context;
	if (l) {
		bcp_vicp_slice_arrived(l->slicer, buf, len);
	}
}

int bcp_vicp_slice_regist_data_arrived_callback(bcp_vicp_receiver_t *r)
{
	return bcp_vicp_receiver_data_arrived_callback(r, 
		data_arrived_callback, r->listener);
}


bcp_vicp_slicer_t *bcp_vicp_slice_create(void *listener)
{
	bcp_vicp_slicer_t *s;

	s = (bcp_vicp_slicer_t*)malloc(sizeof(*s));
	if (!s) {
		return NULL;
	}

	memset(s, 0, sizeof(*s));

	s->stop = 1;
	s->listener = listener;

	s->mutex = Thread_create_mutex();
	if (!s->mutex) {
		goto __failed;
	}
	s->send_mutex = Thread_create_mutex();
	if (!s->send_mutex) {
		goto __failed;
	}
	s->recv_mutex = Thread_create_mutex();
	if (!s->recv_mutex) {
		goto __failed;
	}

	s->sem = Thread_create_sem();
	if (!s->sem) {
		goto __failed;
	}

	ListZero(&s->sending);
	ListZero(&s->received);

	return s;

__failed:
	if (s) {
		if (s->mutex)
			Thread_destroy_mutex(s->mutex);
		if (s->send_mutex)
			Thread_destroy_mutex(s->send_mutex);
		if (s->recv_mutex)
			Thread_destroy_mutex(s->recv_mutex);
		if (s->sem)
			Thread_destroy_sem(s->sem);
		free(s);
	}
	return NULL;
}

int bcp_vicp_slice_start(bcp_vicp_slicer_t *s)
{
	vicp_listener_t *l;

	if (!s) {
		return -1;
	}

	mutex_lock(s->mutex);

	if (!s->stop) {
		mutex_unlock(s->mutex);
		return -1;
	}

	Thread_start(slice_thread, s);

	/* waiting already started */
	while (s->stop != 0) {
		mutex_unlock(s->mutex);
		msleep(100);
		mutex_lock(s->mutex);
	}

	/* all data trans by slice */
	l = (vicp_listener_t*)s->listener;
	bcp_vicp_slice_regist_data_arrived_callback(l->receiver);

	mutex_unlock(s->mutex);

	return 0;
}

int bcp_vicp_slice_stop(bcp_vicp_slicer_t *s)
{
	if (!s) {
		return -1;
	}

	mutex_lock(s->mutex);

	if (s->stop) {
		mutex_unlock(s->mutex);
		return -1;
	}

	s->stop = 1;

	/* waiting already stoped */
	while (s->stop != 2) {
		mutex_unlock(s->mutex);
		msleep(100);
		mutex_lock(s->mutex);
	}

	mutex_unlock(s->mutex);
	return 0;
}

void bcp_vicp_slice_destroy(bcp_vicp_slicer_t *s)
{
	bcp_vicp_slice_stop(s);

	destroy_slice_list(&s->sending, s->send_mutex);
	destroy_slice_list(&s->received, s->recv_mutex);

	Thread_destroy_mutex(s->mutex);
	Thread_destroy_mutex(s->recv_mutex);
	Thread_destroy_mutex(s->send_mutex);
	Thread_destroy_sem(s->sem);

	free(s);
}
