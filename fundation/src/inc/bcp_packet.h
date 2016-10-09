#ifndef __BCP_PACKET_H__
#define __BCP_PACKET_H__

#include "./fundation.h"
#include "./util/LinkedList.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef List list_t;
struct bcp_packet_s;
struct bcp_message_s;

typedef struct bcp_datagram_header_s {
	u8 sof[4];
	u8 version;
	u32 packet_len;
} bcp_datagram_header_t;

typedef struct bcp_datagram_end_s {
	u32 crc32;
	u8 eof[4];
} bcp_datagram_end_t;

typedef struct bcp_element_s {
	u32 len;
	u8 *data;
	struct bcp_message_s *m;
} bcp_element_t;

typedef struct bcp_application_header_s {
	u16 id;
	u8 step_id;
	u8 version;
	u8 session_id;
	u64 sequence_id;
	u32 message_len;
} bcp_application_header_t;

typedef struct bcp_message_s {
	bcp_application_header_t hdr;
	List elements;
	struct bcp_packet_s *p;
} bcp_message_t;

typedef struct bcp_packet_s {
	bcp_datagram_header_t hdr;
	List messages;
	bcp_datagram_end_t end;
} bcp_packet_t;

typedef void bcp_element_foreach_callback_t(bcp_element_t *e, void *context);
typedef void bcp_message_foreach_callback_t(bcp_message_t *m, void *context);

void bcp_packet_init(void);
void bcp_packet_uninit(void);

bcp_packet_t *bcp_packet_create(u8 version);
void bcp_packet_destroy(bcp_packet_t *p);

bcp_message_t *bcp_message_create(u16 application_id, 
	u8 step_id, u8 version, u8 session_id);
void bcp_message_append(bcp_packet_t *p, bcp_message_t *m);
void bcp_message_destroy(bcp_message_t *m);

bcp_element_t *bcp_element_create(u8 *data, u32 len);
void bcp_element_append(bcp_message_t *m, bcp_element_t *e);
void bcp_element_destroy(bcp_element_t *e);

void bcp_messages_foreach(bcp_packet_t *p, bcp_message_foreach_callback_t *cb, void *context);
void bcp_elements_foreach(bcp_message_t *m, bcp_element_foreach_callback_t *cb, void *context);

/*
 * serialize packet
 * params
 *	p be serialized packet
 *	buf serialized buf
 *	len serialized buf length
 * return
 *  return 0 if success or < 0
 */
s32 bcp_packet_serialize(bcp_packet_t *p, u8 **buf, u32 *len);
/*
* unserialize packet
* formating buf to bcp_packet_t struct
* params
*	buf unserialized buf, completely serialized buf
*	len unserialized buf length
*   p   output packet
* return
*  return 0 if success or < 0
*/
s32 bcp_packet_unserialize(u8 *buf, u32 len, bcp_packet_t **p);

#ifdef __cplusplus
}
#endif

#endif // __BCP_PACKET_H__
