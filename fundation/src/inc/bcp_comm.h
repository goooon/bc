#ifndef __BCP_COMM_H__
#define __BCP_COMM_H__

#include "./fundation.h"
#include "./bcp_packet.h"
#include "./util/LinkedList.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void bcp_connected(void *context);
typedef void bcp_connect_failed(void *context);
typedef void bcp_disconnected(void *context);
typedef void bcp_packet_arrived(void *context, bcp_packet_t *p);
typedef void bcp_packet_delivered(void *context, int token);
typedef void bcp_packet_deliver_failed(void *context, int token);

typedef void bcp_subscribe(void *context, char *topic);
typedef void bcp_subscribe_failed(void *context, char *topic);
typedef void bcp_unsubscribe(void *context, char *topic);
typedef void bcp_unsubscribe_failed(void *context, char *topic);

typedef struct bcp_conn_callbacks_s {
	bcp_connected *on_connected;
	bcp_connect_failed *on_connect_failed;
	bcp_disconnected *on_disconnected;
	
	bcp_packet_arrived *on_packet_arrived;
	
	bcp_packet_delivered *on_packet_delivered;
	bcp_packet_deliver_failed *on_packet_deliver_failed;

	bcp_subscribe *on_subscribe;
	bcp_subscribe_failed *on_subscribe_failed;

	bcp_unsubscribe *on_unsubscribe;
	bcp_unsubscribe_failed *on_unsubscribe_failed;
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

int bcp_conn_subscribe(void *hdl, const char *topic, void *context);
int bcp_conn_unsubscribe(void *hdl, const char *topic, void *context);
int bcp_conn_publish_raw(void *hdl, const char *buf, int len, 
	const char *topic, void *context);
int bcp_conn_pulish(void *hdl, bcp_packet_t *p, 
	const char *topic, void *context);

#ifdef __cplusplus
}
#endif

#endif // __BCP_COMM_H__
