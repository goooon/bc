#ifndef __BCP_VICP_PACKET_H__
#define __BCP_VICP_PACKET_H__

#include "../dep.h"
#include "../fundation.h"
#include "../util/Thread.h"
#include "../util/LinkedList.h"

#define VICP_PACKET_TAG 0x12FE6590
#define VICP_PACKET_VERSION 1

/* packet type */
#define VICP_PACKET_REQ 1
#define VICP_PACKET_ACK 2

/* packet tag size */
#define VICP_TAG_SIZE 4

typedef struct vicp_ack_s {
	u8 result;
	u16 code;
} vicp_ack_t;

typedef struct bcp_vicp_packet_s {
	u32 tag;
	u8 type;
	u8 ver;
	u32 msg_id;
	u8 *data;
	u16 len; /* data length */
	u32 crc;
} bcp_vicp_packet_t;

/* packet system init */
void bcp_vicp_packet_init(void);
void bcp_vicp_packet_uninit(void);

u32 bcp_vicp_next_seq_id(void);

int bcp_vicp_packet_header_size(void);
int bcp_vicp_packet_footer_size(void);

bcp_vicp_packet_t *bcp_vicp_packet_create(u8 type, 
	u32 msg_id, u8 *data, u16 len);
void bcp_vicp_packet_destroy(bcp_vicp_packet_t *p);

bcp_vicp_packet_t *bcp_vicp_create_request(u32 msg_id, 
	u8 *data, u16 len);
bcp_vicp_packet_t *bcp_vicp_create_ack(u32 msg_id, 
	u8 result, u16 code);

int bcp_vicp_packet_serialize(bcp_vicp_packet_t *p, 
	u8 **buf, u32 *len);
bcp_vicp_packet_t *bcp_vicp_packet_unserialize(u8 *buf, u32 len);

int bcp_vicp_packet_unserialize_ack(bcp_vicp_packet_t *p, 
	vicp_ack_t *ack);

#endif // __BCP_VICP_PACKET_H__