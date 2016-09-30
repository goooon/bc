#ifndef __BCP_PACKET_H__
#define __BCP_PACKET_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "./dep.h"
#include "./LinkedList.h"

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
	u8 sequence_id;
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

#ifdef __cplusplus
}
#endif

#endif // __BCP_PACKET_H__
