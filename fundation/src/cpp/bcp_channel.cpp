#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <time.h>
#if defined(_WIN32) || defined(_WIN64)
#define __WINOS__ 1
#include <windows.h>
#else
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#endif

#include "../inc/dep.h"
#include "../inc/util/LinkedList.h"
#include "../inc/util/Thread.h"
#include "../inc/bcp_serial.h"
#include "../inc/bcp_channel.h"

static List channels;
static mutex_type mutex = NULL;

void bcp_channel_init(void)
{
	ListZero(&channels);
	mutex = Thread_create_mutex();
	if (!mutex) {
		LOG_W("bcp_channel_init create mutex failed");
	}
}

void bcp_channel_uninit(void)
{
	if (channels.count > 0) {
		LOG_E("bcp_channel_uninit has channels in the list.");
		return;
	}
	ListEmpty(&channels);
	Thread_destroy_mutex(mutex);
	mutex = NULL;
}

static void mutex_lock(mutex_type mutex)
{
	Thread_lock_mutex(mutex);
}

static void mutex_unlock(mutex_type mutex)
{
	Thread_unlock_mutex(mutex);
}

static bcp_channel_t *find_channel(bcp_channel_t *c)
{
	ListElement *e;

	e = ListFind(&channels, (void*)c);
	if (e) {
		return (bcp_channel_t*)e->content;
	}
	return NULL;
}

static bcp_channel_t *find_channel_byname(const char *dev_name)
{
	ListElement *current = NULL;
	bcp_channel_t *c;

	while (ListNextElement(&channels, &current) != NULL) {
		c = (bcp_channel_t*)current->content;
		if (c) {
			if (!strcmp(dev_name, c->dev_name)) {
				return c;
			}
		}
	}

	return NULL;
}

static int insert_channel(bcp_channel_t *c)
{
	if (!find_channel(c)) {
		ListAppend(&channels, (void*)c, sizeof(*c));
		return 0;
	} else {
		return -1;
	}
}

static int remove_channel(bcp_channel_t *c)
{
	if (find_channel(c)) {
		ListDetach(&channels, (void*)c);
		return 0;
	} else {
		return -1;
	}
}

static char *my_strdup(const char *s)
{
	if (!s) {
		return NULL;
	}
#if defined(_WIN32) || defined(_WIN64)
	return _strdup(s);
#else
	return strdup(s);
#endif
}

static int serial_open(void *channel)
{
	bcp_channel_t *c = (bcp_channel_t*)channel;

	if (!c) {
		return -1;
	}

	mutex_lock(c->mutex);

	if (c->hdl) {
		c->hdl_ref++;
		mutex_unlock(c->mutex);
		return 0;
	}

	c->hdl = bcp_serial_open(c->dev_name,
		115200, 8, P_NONE, 1);
	if (!c->hdl) {
		mutex_unlock(c->mutex);
		return -1;
	}

	c->hdl_ref++;
	mutex_unlock(c->mutex);

	return 0;
}

static int serial_close(void *channel)
{
	bcp_channel_t *c = (bcp_channel_t*)channel;

	if (!c) {
		return -1;
	}
	
	mutex_lock(c->mutex);

	if (--c->hdl_ref > 0) {
		mutex_unlock(c->mutex);
		return 0;
	}

	if (c->hdl) {
		bcp_serial_close(c->hdl);
		c->hdl = NULL;
	}

	mutex_unlock(c->mutex);

	return 0;
}

static int serial_read(void *channel, char *buf, int len, int timeout/*ms*/)
{
	bcp_channel_t *c = (bcp_channel_t*)channel;
	int ret;

	if (!c || !buf) {
		return -1;
	}
	
	mutex_lock(c->mutex);
	if (!c->hdl) {
		mutex_unlock(c->mutex);
		return -1;
	}

	c->hdl_ref++;
	mutex_unlock(c->mutex);

	ret = bcp_serial_read(c->hdl, buf, len, timeout);

	mutex_lock(c->mutex);
	c->hdl_ref--;
	mutex_unlock(c->mutex);

	return ret;
}

static int serial_write(void *channel, const char *buf, int len)
{
	bcp_channel_t *c = (bcp_channel_t*)channel;
	int ret;

	if (!c || !buf) {
		return -1;
	}
	
	mutex_lock(c->mutex);
	if (!c->hdl) {
		mutex_unlock(c->mutex);
		return -1;
	}

	c->hdl_ref++;
	mutex_unlock(c->mutex);

	ret = bcp_serial_write(c->hdl, buf, len);

	mutex_lock(c->mutex);
	c->hdl_ref--;
	mutex_unlock(c->mutex);

	return ret;
}

static void set_callback(bcp_channel_t *c)
{
	switch(c->type) {
		case BCP_CHANNEL_SERIAL:
			c->open = serial_open;
			c->close = serial_close;
			c->read = serial_read;
			c->write = serial_write;
			c->packet_size = 256;
			break;
		default:
			LOG_W("bcp channel set callback, unkown type = %d\n", c->type);
			break;
	}
}

bcp_channel_t *bcp_channel_create(int type, const char *dev_name)
{
	bcp_channel_t *c;

	mutex_lock(mutex);

	c = find_channel_byname(dev_name);
	if (c) {
		c->channel_ref++;
		mutex_unlock(mutex);
		return c;
	}

	c = (bcp_channel_t*)malloc(sizeof(*c));
	if (!c) {
		goto __failed;
	}

	memset(c, 0, sizeof(*c));
	c->dev_name = my_strdup(dev_name);
	if (!c->dev_name) {
		goto __failed;
	}
	c->mutex = Thread_create_mutex();
	if (!c->mutex) {
		goto __failed;
	}

	c->type = type;
	c->hdl_ref = 0;
	c->channel_ref = 1;
	c->hdl = NULL;
	set_callback(c);

	if (insert_channel(c) < 0) {
		goto __failed;
	}

	mutex_unlock(mutex);
	return c;

__failed:
	if (c) {
		if (c->dev_name)
			free(c->dev_name);
		Thread_destroy_mutex(c->mutex);
		free(c);
	}
	mutex_unlock(mutex);
	return NULL;
}

void bcp_channel_destroy(bcp_channel_t *c)
{
	if (!c) {
		return;
	}

	mutex_lock(mutex);
	mutex_lock(c->mutex);

	if (c->hdl_ref > 0) {
		mutex_unlock(c->mutex);
		mutex_unlock(mutex);
		return;
	} else if (--c->channel_ref > 0) {
		mutex_unlock(c->mutex);
		mutex_unlock(mutex);
		return;
	}

	mutex_unlock(c->mutex);

	if (remove_channel(c) < 0) {
		LOG_W("bcp_channel_destroy remove_channel failed.\n");
	}

	mutex_unlock(mutex);

	if (c->dev_name)
		free(c->dev_name);
	Thread_destroy_mutex(c->mutex);
	free(c);
}

bcp_channel_t *bcp_channel_get_byname(int type, const char *dev_name)
{
	return bcp_channel_create(type, dev_name);
}

bcp_channel_t* bcp_channel_get(bcp_channel_t *c)
{
	if (!c) {
		return NULL;
	} else {
		mutex_lock(mutex);
		c->channel_ref++;
		mutex_unlock(mutex);
		return c;
	}
}

void bcp_channel_put(bcp_channel_t *c)
{
	if (!c) {
		return;
	} else {
		mutex_lock(mutex);
		if (c->channel_ref == 0) {
			LOG_E("bcp_channel_put channel_ref == 0.\n");
			mutex_unlock(mutex);
		}
		c->channel_ref--;
		mutex_unlock(mutex);
		if (c->channel_ref == 0) {
			bcp_channel_destroy(c);
		}
	}
}
