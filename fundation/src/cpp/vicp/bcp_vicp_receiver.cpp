#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "../../inc/dep.h"
#include "../../inc/util/Thread.h"
#include "../../inc/util/LinkedList.h"
#include "../../inc/util/Timestamp.h"

#include "../../inc/bcp_channel.h"
#include "../../inc/binary_formater.h"

#include "../../inc/vicp/bcp_vicp_packet.h"
#include "../../inc/vicp/bcp_vicp_receiver.h"
#include "../../inc/vicp/bcp_vicp_sender.h"
#include "../../inc/vicp/bcp_vicp.h"

#if !defined(_WIN32) && !defined(_WIN64)
#include <unistd.h>
#else
#include <windows.h>
#endif

static int read_left(bcp_channel_t *c, u8 *buf, int len)
{
	int ret;
	int reads = 0;

	while (reads != len) {
		ret = c->read(c, (char*)buf + reads, len - reads, 1000);
		if (ret > 0) {
			reads += ret;
		} else if (ret < 0) {
			LOG_E("read_left failed, ret=%d\n", ret);
			return reads;
		}
	}

	return reads;
}

static void clear_last_bytes(bcp_vicp_receiver_t *r)
{
	memset(&r->last_bytes[0], 0, sizeof(r->last_bytes));
	r->has_bytes = 0;
}

static void remove_first_bytes(bcp_vicp_receiver_t *r)
{
	int i;

	if (r->has_bytes == 4) {
		for (i = 1; i < 4; i++) {
			r->last_bytes[i - 1] = r->last_bytes[i];
		}
		r->has_bytes--;
	}
}

static u32 read_tag(bcp_vicp_receiver_t *r, 
	bcp_channel_t *c, u8 *header)
{
	int reads;
	u32 tag = 0;

	remove_first_bytes(r);

	reads = r->has_bytes;
	if (reads > 0) {
		memcpy(header, &r->last_bytes[0], reads);
	}
	reads += read_left(c, header + reads, 4 - reads);
	memcpy(&r->last_bytes[0], header, reads);
	r->has_bytes = reads;

	if (reads == 4) {
		tag = header[0] << 24;
		tag |= header[1] << 16;
		tag |= header[2] << 8;
		tag |= header[3] << 0;
	}

	return tag;
}

static u8 *read_whole_block(bcp_channel_t *c, 
	bcp_vicp_receiver_t *r, int *size)
{
	int ret, left, total_size, header_size, footer_size;
	u16 remaing_len;
	u8 *header, *data;
	u32 tag;

	header_size = bcp_vicp_packet_header_size();
	footer_size = bcp_vicp_packet_footer_size();

	header = (u8*)malloc(header_size);
	if (!header) {
		return NULL;
	}

	/* read tag */
	tag = read_tag(r, c, &header[0]);
	if (tag != VICP_PACKET_TAG) {
		//LOG_E("read tag invalid, tag = 0x%x.\n", tag);
		free(header);
		return NULL;
	}

	clear_last_bytes(r);

	/* read left header */
	left = header_size - 4/* tag size */;
	ret = read_left(c, &header[4], left);
	if (ret != left) {
		free(header);
		LOG_E("read left header failed, ret = %d.\n", ret);
		return NULL;
	}

	/* parse remaing data leng */
	remaing_len = header[9] << 8;
	remaing_len |= header[10] << 0;

	total_size = header_size + remaing_len + footer_size;
	data = (u8*)malloc(total_size);
	if (!data) {
		free(header);
		return NULL;
	}

	memcpy(data, header, header_size);
	free(header);

	left = total_size - header_size;
	ret = read_left(c, &data[header_size], left);
	if (ret != left) {
		free(data);
		LOG_E("read left data failed, ret = %d.\n", ret);
		return NULL;
	}

	*size = total_size;
	return data;
}

#ifdef VICP_MOCK_TEST
bcp_vicp_packet_t *read_packet_from_list(void);
#endif

static bcp_vicp_packet_t *read_one_packet(bcp_vicp_receiver_t *r)
{
	bcp_channel_t *c;
	bcp_vicp_packet_t *p;
	int total_size;
	u8 *data;

	c = bcp_vicp_get_channel(r->listener);
	if (!c) {
		LOG_E("bcp_vicp_get_channel failed at read_on_packet.\n");
		return NULL;
	}

#ifdef VICP_MOCK_TEST
	p = read_packet_from_list();
#else
	data = read_whole_block(c, r, &total_size);
	if (!data) {
		return NULL;
	}

	p = bcp_vicp_packet_unserialize(data, total_size);
	if (!p) {
		free(data);
		LOG_E("vicp packet unserialize failed.\n");
		return NULL;
	}
	free(data);
#endif

	return p;
}

static void receiver_ack_callback(void *context, int result)
{
	//LOG_E("receiver_ack_callback. result=%d\n", result);
}

static bcp_vicp_sender_t *get_sender(bcp_vicp_receiver_t *r)
{
	vicp_listener_t *l;

	l = (vicp_listener_t*)r->listener;
	if (!l) {
		return NULL;
	} else {
		return l->sender;
	}
}

static int send_ack(bcp_vicp_receiver_t *r, 
	u8 result, u16 code)
{
	return bcp_vicp_send_ack(get_sender(r), result, code, 
		10 * 1000, receiver_ack_callback, NULL, NULL);
}

static int notify_ack(bcp_vicp_receiver_t *r, 
	bcp_vicp_packet_t *p)
{
	vicp_ack_t ack;

	if (bcp_vicp_packet_unserialize_ack(p, &ack) < 0) {
		return -1;
	}

	bcp_vicp_sender_notify_ack(get_sender(r),
		p->msg_id, ack.result);

	return 0;
}

static int notify_data(bcp_vicp_receiver_t *r, 
	bcp_vicp_packet_t *p)
{
	u8 *buf;

	if (!r->dac) {
		return -1;
	}

	buf = (u8*)malloc(p->len);
	if (!buf) {
		LOG_E("dispatch_packet malloc failed.\n");
		return -1;
	}

	memcpy(buf, p->data, p->len);
	(*r->dac)(r->context, buf, p->len);

	return 0;
}

static int dispatch_packets(bcp_vicp_receiver_t *r)
{
	bcp_vicp_packet_t *p;

	mutex_lock(r->list_mutex);

	while ((p = (bcp_vicp_packet_t*)ListDetachHead(&r->received_list)) != NULL) {
		mutex_unlock(r->list_mutex);
		if (p->type == VICP_PACKET_ACK) {
			notify_ack(r, p);
		} else {
			send_ack(r, 0, 0);
			notify_data(r, p);
		}
		bcp_vicp_packet_destroy(p);
		mutex_lock(r->list_mutex);
	}

	mutex_unlock(r->list_mutex);

	return 0;
}

static void destroy_received_packets(bcp_vicp_receiver_t *r)
{
	bcp_vicp_packet_t *e;

	mutex_lock(r->list_mutex);

	while ((e = (bcp_vicp_packet_t*)ListDetachHead(&r->received_list)) != NULL) {
		bcp_vicp_packet_destroy(e);
	}

	mutex_unlock(r->list_mutex);
}

static void add_packet(bcp_vicp_receiver_t *r,
	bcp_vicp_packet_t *p)
{
	mutex_lock(r->list_mutex);
	ListAppend(&r->received_list, p, sizeof(*p));
	mutex_unlock(r->list_mutex);
}

static thread_return_type WINAPI receiver_thread(void *arg)
{
	bcp_vicp_packet_t *p;
	bcp_vicp_receiver_t *r = (bcp_vicp_receiver_t*)arg;

	if (!r) {
		return NULL;
	}

	mutex_lock(r->mutex);
	r->stop = 0;

	while (!r->stop) {
		mutex_unlock(r->mutex);
		p = read_one_packet(r);
		if (p) {
			add_packet(r, p);
			Thread_post_sem(r->sem);
		}
		mutex_lock(r->mutex);
	}

	destroy_received_packets(r);

	r->stop = 2; /* notify waiting thread */
	mutex_unlock(r->mutex);

	return NULL;
}

static thread_return_type WINAPI dispatch_thread(void *arg)
{
	bcp_vicp_receiver_t *r = (bcp_vicp_receiver_t*)arg;

	if (!r) {
		return NULL;
	}

	mutex_lock(r->mutex);
	r->post_stop = 0;

	while (!r->post_stop) {
		mutex_unlock(r->mutex);
		Thread_wait_sem(r->sem, 1 * 1000);
		dispatch_packets(r);
		mutex_lock(r->mutex);
	}

	destroy_received_packets(r);

	r->post_stop = 2; /* notify waiting thread */
	mutex_unlock(r->mutex);

	return NULL;
}

bcp_vicp_receiver_t *bcp_vcip_receiver_create(void *listener)
{
	bcp_vicp_receiver_t *r;

	r = (bcp_vicp_receiver_t*)malloc(sizeof(*r));
	if (!r) {
		return NULL;
	}

	memset(r, 0, sizeof(*r));

	r->stop = 1;
	r->post_stop = 1;
	r->listener = listener;
	r->mutex = Thread_create_mutex();
	if (!r->mutex) {
		goto __failed;
	}
	r->list_mutex = Thread_create_mutex();
	if (!r->list_mutex) {
		goto __failed;
	}
	r->sem = Thread_create_sem();
	if (!r->sem) {
		goto __failed;
	}
	ListZero(&r->received_list);

	r->has_bytes = 0;
	memset(&r->last_bytes, 0, sizeof(r->last_bytes));
	return r;

__failed:
	if (r) {
		if (r->mutex)
			Thread_destroy_mutex(r->mutex);
		if (r->list_mutex)
			Thread_destroy_mutex(r->list_mutex);
		if (r->sem)
			Thread_destroy_sem(r->sem);
		free(r);
	}
	return NULL;
}

static int start_thread(bcp_vicp_receiver_t *r, 
	int *stop, thread_fn fn)
{
	mutex_lock(r->mutex);

	if (!*stop) {
		LOG_W("vicp receiver thread has been started\n");
		mutex_unlock(r->mutex);
		return -1;
	}

	Thread_start(fn, r);

	/* waiting already started */
	while (*stop != 0) {
		mutex_unlock(r->mutex);
		msleep(100);
		mutex_lock(r->mutex);
	}

	mutex_unlock(r->mutex);
	return 0;
}

int bcp_vicp_receiver_start(bcp_vicp_receiver_t *r)
{
	if (!r) {
		return -1;
	}

	start_thread(r, &r->stop, receiver_thread);
	start_thread(r, &r->post_stop, dispatch_thread);

	return 0;
}

int bcp_vicp_receiver_data_arrived_callback(bcp_vicp_receiver_t *r, 
	fdata_arrived_callback dac, void *context)
{
	if (!r) {
		return -1;
	}

	mutex_lock(r->mutex);
	r->dac = dac;
	r->context = context;
	mutex_unlock(r->mutex);

	return 0;
}

static int stop_thread(bcp_vicp_receiver_t *r,
	int *stop)
{

	mutex_lock(r->mutex);

	if (*stop) {
		LOG_W("vicp receiver thread has stoped\n");
		mutex_unlock(r->mutex);
		return -1;
	}

	*stop = 1;

	/* waiting already stoped */
	while (*stop != 2) {
		mutex_unlock(r->mutex);
		msleep(100);
		mutex_lock(r->mutex);
	}

	mutex_unlock(r->mutex);
	return 0;
}

int bcp_vicp_receiver_stop(bcp_vicp_receiver_t *r)
{
	if (!r) {
		return -1;
	}

	stop_thread(r, &r->stop);
	stop_thread(r, &r->post_stop);

	return 0;
}

void bcp_vicp_receiver_destroy(bcp_vicp_receiver_t *r)
{
	if (r) {
		if (!r->stop || !r->post_stop) {
			bcp_vicp_receiver_stop(r);
		}
		Thread_destroy_mutex(r->mutex);
		free(r);
	}
}
