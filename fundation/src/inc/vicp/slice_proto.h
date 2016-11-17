#ifndef __SLICE_PROTO_H__
#define __SLICE_PROTO_H__

#include "../dep.h"
#include "../fundation.h"
#include "../binary_formater.h"

#define VICP_SLICE_VERSION 0

#define VICP_SLICE_GROUP_ID 1
#define VICP_SLICE_GROUP_ID_ACK 2
#define VICP_SLICE_DESC 3
#define VICP_SLICE_DESC_ACK 4
#define VICP_SLICE_DATA 5
#define VICP_SLICE_DATA_ACK 6

#define VICP_SLICE_ONLY_ONE 7

typedef struct slice_group_s {
	u8 type;
	u8 ver;
	u32 context_id;
} slice_group_t;

typedef struct slice_group_ack_s {
	u8 type;
	u8 ver;
	u32 context_id;
	u32 group_id;
	u16 slice_size; /* per slice max size */
} slice_group_ack_t;

typedef struct slice_desc_s {
	u8 type;
	u8 ver;
	u32 context_id;
	u16 len;
	u32 group_id;
	u16 slice_count;
} slice_desc_t;

typedef struct slice_desc_ack_s {
	u8 type;
	u8 ver;
	u32 context_id;
	u32 group_id;
	u8 result;
} slice_desc_ack_t;

typedef struct slice_data_s {
	u8 type;
	u8 ver;
	u32 context_id;
	u32 group_id;
	u16 slice_id;
	u8 *buf;
	u16 len;
} slice_data_t;

typedef struct slice_data_ack_s {
	u8 type;
	u8 ver;
	u32 context_id;
	u32 group_id;
	u16 slice_id;
	u16 len;
	u8 result;
	u8 code;
} slice_data_ack_t;

/*
 * create_* function return stream at f param
 */
int create_group_id_req(bf_t *f, u32 context_id);

int create_desc_req(bf_t *f,
	u32 context_id, u32 group_id, u16 len, u16 slice_count);

int create_data_req(bf_t *f,
	u32 context_id, u32 group_id, u16 slice_id, u8 *data, u16 len);

int create_group_id_ack(bf_t *f, 
	u32 context_id, u32 group_id, u16 slice_size);

int create_desc_ack(bf_t *f,
	u32 context_id, u32 group_id, u8 result);

int create_data_ack(bf_t *f,
	u32 context_id, u32 group_id, u16 slice_id, 
	u16 len, u8 result, u8 code);

int free_slice_req(bf_t *f);

/*
 * decode struct from stream by f
 */
int parse_group_id_req(bf_t *f, slice_group_t *r);
int parse_desc_req(bf_t *f, slice_desc_t *r);
int parse_data_req(bf_t *f, slice_data_t *r);
int parse_group_id_ack(bf_t *f, slice_group_ack_t *ack);
int parse_desc_ack(bf_t *f, slice_desc_ack_t *ack);
int parse_data_ack(bf_t *f, slice_data_ack_t *ack);

#endif // __SLICE_PROTO_H__
