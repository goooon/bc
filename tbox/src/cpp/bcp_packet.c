#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include "../inc/bcp_packet.h"
#include "../inc/Thread.h"
#include "../inc/crc32.h"

static u32 seq_id = 0;
static mutex_type mutex = NULL;

void bcp_packet_init(void)
{
	mutex = Thread_create_mutex();
}

void bcp_packet_uninit(void)
{
	if (mutex) {
		Thread_destroy_mutex(mutex);
		mutex = NULL;
	}
}

u32 bcp_next_seq_id(void)
{
	u32 id = 0;

	Thread_lock_mutex(mutex);
	id = seq_id++;
	Thread_unlock_mutex(mutex);

	return id;
}

bcp_packet_t *bcp_create_packet(u8 version)
{
	bcp_packet_t *p;
	
	p = (bcp_packet_t*)malloc(sizeof(*p));
	if (!p) {
		return NULL;
	}

	memset(p, 0, sizeof(*p));

	p->hdr.sof[0] = 0x3f;
	p->hdr.sof[1] = 0x6c;
	p->hdr.sof[2] = 0x81;
	p->hdr.sof[3] = 0x29;
	p->hdr.version = version;

	ListZero(&p->messages);

	p->end.crc32 = 0;
	p->end.eof[0] = 0x68;
	p->end.eof[1] = 0x1a;
	p->end.eof[2] = 0x5b;
	p->end.eof[3] = 0x90;

	return p;
}

void bcp_free_elements(List *list)
{
	bcp_element_t *e;
	ListElement* current = NULL;

	while (ListNextElement(list, &current) != NULL) {
		e = current->content;
		if (e) {
			if (e->len > 0 && e->data) {
				free(e->data);
			}
			free(e);
			current->content = NULL;
		}
	}

	ListEmpty(list);
}

void bcp_free_messages(List *list)
{
	bcp_message_t *m;
	ListElement* current = NULL;

	while (ListNextElement(list, &current) != NULL) {
		m = current->content;
		if (m) {
			bcp_free_elements(&m->elements);
			free(m);
			current->content = NULL;
		}
	}

	ListEmpty(list);
}

void bcp_destroy_packet(bcp_packet_t *p)
{
	if (!p) {
		return NULL;
	}

	bcp_free_messages(&p->messages);
	free(p);
}

static u32 element_size(bcp_element_t *e)
{
	if (e) {
		return e->len + 2;
	} else {
		return 0;
	}
}

static u32 messages_size(bcp_message_t *m)
{
	return 15;
}

static u32 datagram_hdr_size(bcp_packet_t *p)
{
	return 8;
}

static u32 datagram_crc_size(bcp_packet_t *p)
{
	return  datagram_hdr_size(p) - sizeof(p->hdr.sof);
}

static u32 datagram_end_size(bcp_packet_t *p)
{
	return 8;
}

bcp_message_t *bcp_create_message(u16 id, u8 step_id, u8 version, u8 session_id)
{
	bcp_message_t *m;

	m = (bcp_message_t*)malloc(sizeof(*m));
	if (!m) {
		return NULL;
	}

	memset(m, 0, sizeof(*m));

	m->hdr.id = id;
	m->hdr.step_id = step_id;
	m->hdr.session_id = session_id;
	m->hdr.version = version;
	m->hdr.sequence_id = bcp_next_seq_id();
	m->hdr.message_len = 0;

	return m;
}

void bcp_append_message(bcp_packet_t *p, bcp_message_t *m)
{
	if (!p || !m) {
		return NULL;
	}

	if (!m->hdr.sequence_id) {
		m->hdr.sequence_id = bcp_next_seq_id();
	}

	ListAppend(&p->messages, m, sizeof(*m));
	p->hdr.packet_len += messages_size(m);
	m->p = p;
}

bcp_element_t *bcp_create_element(u8 *data, u32 len)
{
	bcp_element_t *e;

	e = (bcp_element_t*)malloc(sizeof(*e));
	if (!e) {
		return NULL;
	}

	e->len = len;
	e->data = data;

	return e;
}

void bcp_append_element(bcp_message_t *m, bcp_element_t *e)
{
	u32 sz;

	if (!m || !e) {
		return;
	}

	ListAppend(&m->elements, e, sizeof(*e));

	sz = element_size(e);
	m->hdr.message_len += sz;
	m->p->hdr.packet_len += sz;
	e->m = m;
}

void bcp_calc_crc32(bcp_packet_t *p, u8 *buf, u32 len)
{
	u32 hdrlen, buflen, crc32;
	u8 *start;

	if (!p || !buf) {
		return;
	}

	hdrlen = datagram_crc_size(p);
	start = buf + hdrlen;
	buflen = hdrlen + p->hdr.packet_len;
	crc32 = calc_crc32(start, buflen);

	p->end.crc32 = crc32;
}

static u32 datagram_hdr_serialize(bcp_packet_t *p, u8 *buf, u32 i)
{
	/* sof */
	buf[i++] = p->hdr.sof[0];
	buf[i++] = p->hdr.sof[1];
	buf[i++] = p->hdr.sof[2];
	buf[i++] = p->hdr.sof[3];
	/* version */
	buf[i++] = p->hdr.version & 0xf;
	/* remaing length */
	buf[i++] = ((p->hdr.packet_len) >> 16) & 0xff;
	buf[i++] = ((p->hdr.packet_len) >> 8) & 0xff;
	buf[i++] = ((p->hdr.packet_len) >> 0) & 0xff;

	return i;
}

static u32 application_hdr_serialize(bcp_application_header_t *hdr, u8 *buf, u32 i)
{
	/* application id */
	buf[i++] = (hdr->id >> 8) & 0xff;
	buf[i++] = (hdr->id >> 0) & 0xff;
	buf[i++] = ((hdr->stepid & 0xf) << 4) | (hdr->version & 0xf);
	buf[i++] = (hdr->session_id) & 0xff;

	/* seq id */
	buf[i++] = (hdr->sequence_id >> 24) & 0xff;
	buf[i++] = (hdr->sequence_id >> 16) & 0xff;
	buf[i++] = (hdr->sequence_id >> 8) & 0xff;
	buf[i++] = (hdr->sequence_id >> 0) & 0xff;
	buf[i++] = 0;
	buf[i++] = 0;
	buf[i++] = 0;
	buf[i++] = 0;

	/* remaing length */
	buf[i++] = (hdr->message_len >> 16) & 0xff;
	buf[i++] = (hdr->message_len >> 8) & 0xff;
	buf[i++] = (hdr->message_len >> 0) & 0xff;
	
	return i;
}

static u32 element_serialize(bcp_element_t *e, u8 *buf, u32 i)
{
	buf[i++] = (e->len >> 8) & 0xff;
	buf[i++] = (e->len >> 0) & 0xff;
	if (e->len > 0) {
		memcpy(&buf[i], e->data, e->len);
		i += e->len;
	}
	return i;
}

static u32 elements_serialize(bcp_message_t *m, u8 *buf, u32 i)
{
	bcp_element_t *e;
	ListElement* current = NULL;

	while (ListNextElement(&m->elements, &current) != NULL) {
		e = current->content;
		if (e) {
			i = element_serialize(e, buf, i);
		}
	}

	return i;

}

static u32 messages_serialize(bcp_packet_t *p, u8 *buf, u32 i)
{
	bcp_message_t *m;
	ListElement* current = NULL;

	while (ListNextElement(&p->messages, &current) != NULL) {
		m = current->content;
		if (m) {
			i = application_hdr_serialize(&m->hdr, buf, i);
			i = elements_serialize(m, buf, i);
		}
	}

	return i;
}

static u32 datagram_end_serialize(bcp_packet_t *p, u8 *buf, u32 i)
{
	/* crc32 */
	buf[i++] = ((p->end.crc32) >> 24) & 0xff;
	buf[i++] = ((p->end.crc32) >> 16) & 0xff;
	buf[i++] = ((p->end.crc32) >> 8) & 0xff;
	buf[i++] = ((p->end.crc32) >> 0) & 0xff;
	/* eof */
	buf[i++] = p->end.eof[0];
	buf[i++] = p->end.eof[1];
	buf[i++] = p->hdr.sof[2];
	buf[i++] = p->hdr.sof[3];

	return i;
}

/*
 * serialize packet
 * params
 *	p be serialized packet
 *	buf serialized buf
 *	len serialized buf length
 * return
 *  return 0 if success or -1
 */
u8 bcp_packet_serialize(bcp_packet_t *p, u8 **buf, u32 *len)
{
	u8 *ibuf;
	u32 i, bytes;

	if (!p || !buf) {
		return -1;
	}

	if (!p->hdr.packet_len) {
		return -1;
	}

	bytes = datagram_hdr_size(p);
	bytes += p->hdr.packet_len;
	bytes += datagram_end_size(p);

	ibuf = (u8*)malloc(bytes);
	if (!ibuf) {
		return -1;
	}

	i = 0;
	i = datagram_hdr_serialize(p, ibuf, i);
	i = messages_serialize(p, ibuf, i);
	bcp_calc_crc32(p, ibuf, i);
	i = datagram_end_serialize(p, ibuf, i);

	if (i != p->hdr.packet_len + datagram_crc_size(p)) {
		free(ibuf);
		printf("bcp_packet_serialize failed.\n");
		return -1;
	}

	*buf = ibuf;
	*len = i;

	return 0;
}

static u32 datagram_hdr_unserialize(bcp_packet_t *p, u8 *buf, u32 i, u32 len)
{
	u32 v;

	if (len < i + datagram_hdr_size(p)) {
		return i;
	}

	p->hdr.sof[0] = buf[i++];
	p->hdr.sof[1] = buf[i++];
	p->hdr.sof[2] = buf[i++];
	p->hdr.sof[3] = buf[i++];
	p->hdr.version = buf[i++] & 0xf;

	v = 0;
	v |= buf[i++] << 16;
	v |= buf[i++] << 8;
	v |= buf[i++] << 0;

	p->hdr.packet_len = v;

	return i;
}

static u32 datagram_messages_unserialize(bcp_packet_t *p, u8 *buf, u32 i, u32 len)
{
	
	if (len < i + p->hdr.packet_len) {
		return i;
	}
	return i;
}

/*
* unserialize packet
* format buf to bcp_packet_t struct
* params
*	buf unserialized buf
*	len unserialized buf length
*   p   output packet
* return
*  return 0 if success or -1
*/
u8 bcp_packet_unserialize(u8 *buf, u32 len, bcp_packet_t **p)
{
	u32 i, crc32;
	bcp_packet_t *pk;
	
	if (!buf || !p) {
		return -1;
	}

	pk = bcp_create_packet(0);
	if (!pk) {
		return -1;
	}

	/* validation sof & eof */
	if (!bcmp(&buf[0], &pk->hdr.sof[0], sizeof(pk->hdr.sof) ||
		!bcmp(&buf[len - 8], &pk->end.eof[0], sizeof(pk->end.eof)) {
		bcp_destroy_packet(p);
		return -1;
	}

	i = 0;
	crc32 = 0;

	i = datagram_hdr_unserialize(pk, buf, i, len);
	i = datagram_messages_unserialize(pk, buf, i, len);
	i = datagram_end_unserialize(pk, buf, i, len);

	if (len > datagram_crc_size(p) + p->hdr.packet_len) {
		crc32 = bcp_calc_crc32(p, buf + sizeof(p->hdr.sof), 
			datagram_crc_size(p) + p->hdr.packet_len);
		if (crc32 != pk->end.crc32) {
			bcp_destroy_packet(p);
			printf("bcp_packet_unserialize crc32 failed.\n");
			return -1;
		}
	}

	*p = pk;

	return 0;
}
