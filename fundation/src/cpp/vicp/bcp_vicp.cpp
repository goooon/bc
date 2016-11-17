#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "../../inc/dep.h"
#include "../../inc/util/Thread.h"
#include "../../inc/util/LinkedList.h"

#include "../../inc/bcp_channel.h"

#include "../../inc/vicp/bcp_vicp_packet.h"
#include "../../inc/vicp/bcp_vicp_receiver.h"
#include "../../inc/vicp/bcp_vicp_sender.h"
#include "../../inc/vicp/bcp_vicp_slice.h"
#include "../../inc/vicp/bcp_vicp.h"

#if !defined(_WIN32) && !defined(_WIN64)
#include <unistd.h>
#else
#include <windows.h>
#endif

static List listeners;
static mutex_type mutex = NULL;

void bcp_vicp_init(void)
{
	ListZero(&listeners);
	listeners = Thread_create_mutex();
	if (!listeners) {
		LOG_W("bcp_vicp_init create mutex failed");
	}
	bcp_vicp_packet_init();
	bcp_vicp_sender_init();
	bcp_vicp_slice_init();
}

void bcp_vicp_uninit(void)
{
	if (listeners.count > 0) {
		LOG_E("bcp_vicp_uninit has listeners in the list.");
		return;
	}
	ListEmpty(&listeners);
	Thread_destroy_mutex(mutex);
	mutex = NULL;

	bcp_vicp_slice_uninit();
	bcp_vicp_sender_uninit();
	bcp_vicp_packet_uninit();
}

static vicp_listener_t *find_listener(vicp_listener_t *c)
{
	ListElement *e;
	vicp_listener_t *c = NULL;

	e = ListFind(&listeners, (void*)c);
	if (e) {
		c = (vicp_listener_t*)e->content;
	}

	return c;
}

static vicp_listener_t *find_listener_bychannel(bcp_channel_t *c)
{
	ListElement *current = NULL;
	vicp_listener_t *listener;

	while (ListNextElement(&listeners, &current) != NULL) {
		listener = (vicp_listener_t*)current->content;
		if (listener && listener->ch == c) {
			return listener;
		}
	}

	return NULL;
}

static int insert_listener(vicp_listener_t *c)
{
	if (!find_channel(c) 
		&& ListAppend(&listeners, (void*)c, sizeof(*c))) {
		return 0;
	} else {
		return -1;
	}
}

static int remove_listener(vicp_listener_t *c)
{
	if (ListDetach(&listeners, (void*)c)) {
		return 0;
	}
	return -1;
}

void bcp_vicp_data_arrived(void *listener, 
	u8 *buf, u16 len)
{
	vicp_listener_t *lsner = (vicp_listener_t*)listener;

	if (lsner && lsner->vdac) {
		(*lsner->vdac)(lsner->context, buf, len);
	} else {
		free(buf);
	}
}

int bcp_vicp_regist_channel(bcp_channel_t *c)
{
	int ret = -1;
	vicp_listener_t *listener = NULL;

	mutex_lock(mutex);
	if (find_listener_bychannel(c)) {
		mutex_unlock(mutex);
		return -1;
	}

	listener = (vicp_listener_t*)malloc(sizeof(*listener));
	if (!listener) {
		goto __failed;
	}

	memset(listener, 0, sizeof(*listener));

	listener->ch = c;
	listener->ref = 0;
	listener->mutex = Thread_create_mutex();
	if (!listener->mutex) {
		goto __failed;
	}

	if (!(listener->sender = bcp_vicp_sender_create(listener))) {
		goto __failed;
	}
	if (!(listener->receiver = bcp_vcip_receiver_create(listener))) {
		goto __failed;
	}
	if (!(listener->slicer = bcp_vicp_slice_create(listener))) {
		goto __failed;
	}

	bcp_vicp_sender_start(listener->sender);
	bcp_vicp_receiver_start(listener->receiver);
	bcp_vicp_slice_start(listener->slicer);

	c->listener = listener;
	bcp_vicp_get_channel(listener);

	insert_listener(listener);

	ret = 0;

__failed:
	if (listener) {
		if (listener->receiver)
			bcp_vicp_receiver_destroy(listener->receiver);
		if (listener->sender)
			bcp_vicp_sender_destroy(listener->sender);
		if (listener->slicer)
			bcp_vicp_slice_destroy(listener->slicer);
		if (listener->mutex)
			Thread_destroy_mutex(listener->mutex);
		free(listener);
	}
	mutex_unlock(mutex);

	return ret;
}

int bcp_vicp_unregist_channel(bcp_channel_t *c)
{
	vicp_listener_t *listener;

	if (!c || !c->listener) {
		return -1;
	} else {
		listener = c->listener;
	}

	mutex_lock(mutex);

	mutex_lock(listener->mutex);
	if (listener->ref > 0) {
		LOG_W("listener having used, ref=%d.\n", listener->ref);
		mutex_unlock(listener->mutex);
		mutex_unlock(mutex);
		return -1;
	}
	mutex_unlock(listener->mutex);

	bcp_vicp_sender_stop(listener->sender);
	bcp_vicp_receiver_stop(listener->receiver);
	bcp_vicp_slice_stop(listener->slicer);

	bcp_vicp_sender_destroy(listener->sender);
	bcp_vicp_receiver_destroy(listener->receiver);
	bcp_vicp_slice_destroy(listener->slicer);

	c->listener = NULL;

	bcp_vicp_put_channel(listener);
	Thread_destroy_mutex(listener->mutex);

	remove_listener(listener);
	free(listener);

	mutex_unlock(mutex);

	return 0;
}

int bcp_vicp_send(bcp_channel_t *c,
	const char *buf, int len, int timeout, 
	vicp_sender_callback complete, void *context, u32 *id)
{
	vicp_listener_t *l;

	if (c) {
		l = (vicp_listener_t*)c->listener;
		if (l) {
			return bcp_vicp_slice_send(l->slicer, buf, len, timeout,
				complete, context, id);
		}
	}

	return -1;
}

int bcp_vicp_regist_data_arrived_callback(
	bcp_channel_t *c,
	vicp_data_arrived_callback cb,
	void *context)
{
	vicp_listener_t *listener;

	if (!c) {
		return -1;
	}

	listener = (vicp_listener_t*)c->listener;
	if (!listener) {
		return -1;
	}

	mutex_lock(listener->mutex);
	listener->vdac = cb;
	listener->context = context;
	mutex_unlock(listener->mutex);

	return 0;
}

void bcp_vicp_get_listener(void *listener)
{
	vicp_listener_t *l = (vicp_listener_t*)listener;

	if (l) {
		mutex_lock(l->mutex);
		l->ref++;
		mutex_unlock(l->mutex);
	}
}

void bcp_vicp_put_listener(void *listener)
{
	vicp_listener_t *l = (vicp_listener_t*)listener;

	if (l) {
		mutex_lock(l->mutex);
		if (--l->ref == 0) {
			mutex_unlock(l->mutex);
			bcp_vicp_unregist_channel(l->ch);
			return;
		}
		mutex_unlock(l->mutex);
	}
}

bcp_channel_t *bcp_vicp_get_channel(void *listener)
{
	vicp_listener_t *l = (vicp_listener_t*)listener;

	if (l && l->ch) {
		return bcp_channel_get(l->ch);
	} else {
		return NULL;
	}
}

void bcp_vicp_put_channel(void *listener)
{
	vicp_listener_t *l = (vicp_listener_t*)listener;

	if (l && l->ch) {
		bcp_channel_put(l->ch);
	}
}

