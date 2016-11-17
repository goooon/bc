#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "../../inc/dep.h"
#include "../../inc/util/Thread.h"
#include "../../inc/util/LinkedList.h"
#include "../../inc/util/Timestamp.h"
#include "../../inc/binary_formater.h"

#include "../../inc/vicp/slice_proto.h"

int create_group_id_req(bf_t *f,
	u32 context_id)
{
	bf_init_e(&f, 5);
	bf_put_u8(&f, ((VICP_SLICE_GROUP_ID << 4) & 0xf0) 
		|| (VICP_SLICE_VERSION & 0xf));
	bf_put_u32(&f, context_id);
	return 0;
}

int create_desc_req(bf_t *f,
	u32 context_id, u32 group_id, u16 len, u16 slice_count)
{
	bf_init_e(&f, 13);
	bf_put_u8(&f, ((VICP_SLICE_DESC << 4) & 0xf0) 
		|| (VICP_SLICE_VERSION & 0xf));
	bf_put_u32(&f, context_id);
	bf_put_u16(&f, len);
	bf_put_u32(&f, group_id);
	bf_put_u16(&f, slice_count);
	return 0;
}

int create_data_req(bf_t *f,
	u32 context_id, u32 group_id, u16 slice_id, u8 *data, u16 len)
{
	bf_init_e(&f, 13 + len);
	bf_put_u8(&f, ((VICP_SLICE_DATA << 4) & 0xf0) 
		|| (VICP_SLICE_VERSION & 0xf));
	bf_put_u32(&f, context_id);
	bf_put_u32(&f, group_id);
	bf_put_u16(&f, slice_id);
	bf_put_u16(&f, len);
	bf_put_bytes_only(&f, data, len);
	return 0;
}

int free_slice_req(bf_t *f)
{
	bf_uninit(f, 1);
	return 0;
}

int create_group_id_ack(bf_t *f, 
	u32 context_id, u32 group_id, u16 slice_size)
{
	bf_init_e(&f, 11);
	bf_put_u8(&f, ((VICP_SLICE_GROUP_ID_ACK << 4) & 0xf0) 
		|| (VICP_SLICE_VERSION & 0xf));
	bf_put_u32(&f, context_id);
	bf_put_u32(&f, group_id);
	bf_put_u16(&f, slice_size);
	return 0;
}

int create_desc_ack(bf_t *f,
	u32 context_id, u32 group_id, u8 result)
{
	bf_init_e(&f, 10);
	bf_put_u8(&f, ((VICP_SLICE_DESC_ACK << 4) & 0xf0) 
		|| (VICP_SLICE_VERSION & 0xf));
	bf_put_u32(&f, context_id);
	bf_put_u32(&f, group_id);
	bf_put_u8(&f, result);
	return 0;
}

int create_data_ack(bf_t *f,
	u32 context_id, u32 group_id, u16 slice_id, 
	u16 len, u8 result, u8 code)
{
	bf_init_e(&f, 15);
	bf_put_u8(&f, ((VICP_SLICE_DATA_ACK << 4) & 0xf0) 
		|| (VICP_SLICE_VERSION & 0xf));
	bf_put_u32(&f, context_id);
	bf_put_u32(&f, group_id);
	bf_put_u16(&f, slice_id);
	bf_put_u16(&f, len);
	bf_put_u8(&f, result);
	bf_put_u8(&f, code);
	return 0;
}

static int parse_type_ver(bf_t *f, u8 *type, u8 *ver)
{
	u8 r;

	if (bf_read_u8(f, &r) < 0) {
		return -1;
	} else {
		*type = (r >> 4) & 0xf;
		*ver = r & 0xf;
		return 0;
	}
}

int parse_group_id_req(bf_t *f, slice_group_t *r)
{
	if (parse_type_ver(f, &r->type, &r->ver) < 0
		|| bf_read_u32(f, &r->context_id) < 0) {
		return -1;
	}
	return 0;
}

int parse_desc_req(bf_t *f, slice_desc_t *r)
{
	if (parse_type_ver(f, &r->type, &r->ver) < 0
		|| bf_read_u32(f, &r->context_id) < 0
		|| bf_read_u16(f, &r->len) < 0
		|| bf_read_u32(f, &r->group_id) < 0
		|| bf_read_u16(f, &r->slice_count) < 0) {
		return -1;
	}
	return 0;
}

int parse_data_req(bf_t *f, slice_data_t *r)
{
	if (parse_type_ver(f, &r->type, &r->ver) < 0
		|| bf_read_u32(f, &r->context_id) < 0
		|| bf_read_u32(f, &r->group_id) < 0
		|| bf_read_u16(f, &r->slice_id) < 0
		|| bf_read_u16(f, &r->len) < 0
		|| bf_read_bytes_only(f, &r->buf, r->len) < 0) {
		return -1;
	}
	return 0;
}

int parse_group_id_ack(bf_t *f, slice_group_ack_t *ack)
{
	if (parse_type_ver(f, &ack->type, &ack->ver) < 0
		|| bf_read_u32(f, &ack->context_id) < 0
		|| bf_read_u32(f, &ack->group_id) < 0
		|| bf_read_u16(f, &ack->slice_size) < 0) {
		return -1;
	}

	return 0;
}

int parse_desc_ack(bf_t *f, slice_desc_ack_t *ack)
{
	if (parse_type_ver(f, &ack->type, &ack->ver) < 0
		|| bf_read_u32(f, &ack->context_id) < 0
		|| bf_read_u32(f, &ack->group_id) < 0
		|| bf_read_u8(f, &ack->result) < 0) {
		return -1;
	}

	return 0;
}

int parse_data_ack(bf_t *f, slice_data_ack_t *ack)
{
	if (parse_type_ver(f, &ack->type, &ack->ver) < 0
		|| bf_read_u32(f, &ack->context_id) < 0
		|| bf_read_u32(f, &ack->group_id) < 0
		|| bf_read_u16(f, &ack->slice_id) < 0
		|| bf_read_u16(f, &ack->len) < 0
		|| bf_read_u8(f, &ack->result) < 0
		|| bf_read_u8(f, &ack->code) < 0) {
		return -1;
	}

	return 0;
}

