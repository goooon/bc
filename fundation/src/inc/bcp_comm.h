#ifndef __BCP_COMM_H__
#define __BCP_COMM_H__

#include "./fundation.h"
#include "./bcp_packet.h"
#include "./util/LinkedList.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void bcp_connected(void *context);
typedef void bcp_disconnected(void *context);
typedef void bcp_packet_arrived(void *context, bcp_packet_t *p);
typedef void bcp_packet_delivered(void *context, int token);

typedef struct bcp_conn_callbacks_s {
	bcp_connected *on_connected;
	bcp_disconnected *on_disconnected;
	bcp_packet_arrived *on_packet_arrived;
	bcp_packet_delivered *on_packet_delivered;
} bcp_conn_callbacks_t;

void bcp_conn_init(void);
void bcp_conn_uninit(void);

void *bcp_conn_create(const char *address, const char *clientid);

void bcp_conn_set_callbacks(void *hdl, bcp_conn_callbacks_t *cbs,
	void *context);
void bcp_conn_set_qos(void *hdl, int qos);
void bcp_conn_set_keepalive(void *hdl, int keepalive_interval);

int bcp_conn_connect(void *hdl);
int bcp_conn_isconnected(void *hdl);
int bcp_conn_disconnect(void *hdl);
void bcp_conn_destroy(void *hdl);

int bcp_conn_subscribe(void *hdl, const char *topic);
int bcp_conn_unsubscribe(void *hdl, const char *topic);
int bcp_conn_publish_raw(void *hdl, const char *topic, const char *buf, int len);
int bcp_conn_pulish(void *hdl, const char *topic, bcp_packet_t *p);

#ifdef __cplusplus
}
#endif

#endif // __BCP_COMM_H__
