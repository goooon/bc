#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include "../../inc/dep.h"
#include "../../inc/crc32.h"
#include "../../inc/util/Thread.h"
#include "../../inc/binary_formater.h"

#include "../../inc/vicp/bcp_vicp_packet.h"

static u32 seq_id = 1;
static mutex_type mutex = NULL;

void bcp_vicp_packet_init(void)
{
	mutex = Thread_create_mutex();
}

void bcp_vicp_packet_uninit(void)
{
	if (mutex) {
		Thread_destroy_mutex(mutex);
		mutex = NULL;
	}
}

u32 bcp_vicp_next_seq_id(void)
{
	u32 id = 0;

	mutex_lock(mutex);
	id = seq_id++;
	mutex_lock(mutex);

	return id;
}

bcp_vicp_packet_t *bcp_vicp_packet_create(u8 type, 
	u32 msg_id, u8 *data, u16 len)
{
	bcp_vicp_packet_t *p;

	p = (bcp_vicp_packet_t*)malloc(sizeof(*p));
	if (!p) {
		return NULL;
	}

	memset(p, 0, sizeof(*p));

	p->tag = VICP_PACKET_TAG;
	p->type = type;
	p->ver = VICP_PACKET_VERSION;
	p->msg_id = msg_id;

	if (data && len > 0) {
		p->data = (u8*)malloc(len);
		if (!p->data) {
			free(p);
			return NULL;
		}
		memcpy(p->data, data, len);
	} else {
		p->data = NULL;
		p->len = 0;
	}
	p->crc = 0;

	return p;
}

static int ack_data_size(void)
{
	return 3;
}

bcp_vicp_packet_t *bcp_vicp_create_request(u32 msg_id, 
	u8 *data, u16 len)
{
	return bcp_vicp_packet_create(VICP_PACKET_REQ,
		msg_id, data, len);
}

bcp_vicp_packet_t *bcp_vicp_create_ack(u32 msg_id, 
	u8 result, u16 code)
{
	bcp_vicp_packet_t *p;
	bf_t f;

	if (bf_init_e(&f, ack_data_size()) < 0) {
		return NULL;
	}

	bf_put_u8(&f, result);
	bf_put_u16(&f, code);

	p = bcp_vicp_packet_create(VICP_PACKET_ACK,
		msg_id, bf_stream(&f), bf_size(&f));

	bf_uninit(&f, 1);

	return p;
}

void bcp_vicp_packet_destroy(bcp_vicp_packet_t *p)
{
	if (!p) {
		return;
	}

	if (p->data) {
		free(p->data);
	}

	free(p);
}

int bcp_vicp_packet_header_size(void)
{
	return 11; /* header size */
}

int bcp_vicp_packet_footer_size(void)
{
	return 4; /* crc size */
}

static int packet_size(bcp_vicp_packet_t *p)
{
	int data_len = 0;

	if (p) {
		data_len = p->len;
	}

	return bcp_vicp_packet_header_size() + data_len; 
		+ bcp_vicp_packet_footer_size();
}

static int serialize_header(bcp_vicp_packet_t *p, 
	bf_t *f)
{
	if (bf_put_u32(f, p->tag) < 0
		|| bf_put_u8(f, ((p->type << 4) & 0xf0) | (p->ver & 0xf)) < 0
		|| bf_put_u32(f, p->msg_id)< 0
		|| bf_put_u16(f, p->len) < 0 /* remaing length */) {
		return -1;
	}

	return 0;
}

static int serialize_data(bcp_vicp_packet_t *p, bf_t *f)
{
	if (p->data && p->len > 0) {
		return bf_put_bytes_only(f, p->data, p->len);
	}  else {
		return 0;
	}
}

static int serialize_footer(bcp_vicp_packet_t *p, bf_t *f)
{
	p->crc = calc_crc32(bf_stream(f), bf_size(f));
	return bf_put_u32(f, p->crc);
}

int bcp_vicp_packet_serialize(bcp_vicp_packet_t *p, 
	u8 **buf, u32 *len)
{
	bf_t f;
	int ret = -1;

	if (!p) {
		return -1;
	}

	if (bf_init_e(&f, packet_size(p)) < 0) {
		return -1;
	}

	if (serialize_header(p, &f) < 0
		|| serialize_data(p, &f) < 0
		|| serialize_footer(p, &f) < 0) {
		goto __failed;
	}

	*buf = bf_stream(&f);
	*len = bf_size(&f);
	ret = 0;

__failed:
	bf_uninit(f, (ret < 0));
	return ret;
}

static bcp_vicp_packet_t *unserialize_header(bf_t *f)
{
	bcp_vicp_packet_t *p;
	u32 tag;
	u8 r, type, ver;
	u32 msg_id;
	u8 *data;
	u16 len;

	if (bf_read_u32(f, &tag) < 0
		|| bf_read_u8(f, &r) < 0) {
		return NULL;
	}

	type = r >> 4 & 0xf;
	ver = r & 0xf;

	if (bf_read_u32(f, &msg_id) < 0
		|| bf_read_u16(f, &len) < 0) {
		return NULL;
	}

	p = bcp_vicp_packet_create(type, msg_id, NULL, 0);
	if (!p) {
		return NULL;
	}

	p->len = len; /* remaing length */
	p->tag = tag;
	p->ver = ver;

	return p;
}

static int unserialize_data(bcp_vicp_packet_t *p, bf_t *f)
{
	if (p->len > 0) {
		return bf_read_bytes_only(f, &p->data, p->len);
	} else {
		return 0;
	}
}

static int unserialize_footer(bcp_vicp_packet_t *p, bf_t *f)
{
	return bf_read_u32(f, &p->crc);
}

static int compare_crc(bcp_vicp_packet_t *p, bf_t *f)
{
	u32 crc;

	crc = calc_crc32(bf_stream(f), bf_size(f) - 4/* crc */);
	if (p->crc != crc) {
		return -1;
	}

	return 0;
}

bcp_vicp_packet_t *bcp_vicp_packet_unserialize(u8 *buf, u32 len)
{
	bcp_vicp_packet_t *p = NULL;
	bf_t f;
	int ret;

	if (!buf) {
		return NULL;
	}

	bf_init_d(&f, buf, len);

	if (!(p = unserialize_header(&f))
		|| unserialize_data(p, &f) < 0
		|| unserialize_footer(p, &f) < 0) {
		goto __failed;
	}

	if (compare_crc(p, &f) < 0) {
		LOG_E("bcp_vicp_packet_unserialize crc failed.\n");
		goto __failed;
	}

	return p;

__failed:
	if (p) {
		bcp_vicp_packet_destroy(p);
	}
	return NULL;
}

int bcp_vicp_packet_unserialize_ack(bcp_vicp_packet_t *p, 
	vicp_ack_t *ack)
{
	bf_t f;

	if (!p || !ack) {
		return -1;
	}

	if (p->type != VICP_PACKET_ACK) {
		return -1;
	}

	bf_init_d(&f, p->data, p->len);

	if (bf_read_u8(&f, &ack->result) < 0 
		|| bf_read_u16(&f, &ack->code) < 0) {
		return -1;
	}

	return 0;
}

