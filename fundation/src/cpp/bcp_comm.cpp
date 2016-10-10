#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "../inc/dep.h"
#include "../inc/bcp_packet.h"
#include "../inc/crc32.h"
#include "../inc/util/Thread.h"
#include "../inc/util/LinkedList.h"

#include "../inc/MQTTAsync.h"
#include "../inc/MQTTClientPersistence.h"

#include "../inc/bcp_comm.h"

#if !defined(WIN32)
#include <unistd.h>
#else
#include <windows.h>
#endif

typedef struct bcp_conn_s {
	mutex_type mutex;
	char *address;
	char *clientid;
	int connected;
	MQTTAsync client;
	MQTTAsync_connectOptions opts;

	int qos;
	void *callback_context;
	bcp_conn_callbacks_t *cbs;

	/* subscribe topics */
	List topics;
} bcp_conn_t;

#define DEF_QOS	1

static char *my_strdup(const char *s)
{
	if (!s) {
		return NULL;
	}
#if defined(WIN32)
	return _strdup(s);
#else
	return strdup(s);
#endif
}

static void conn_mutex_lock(bcp_conn_t *conn)
{
	Thread_lock_mutex(conn->mutex);
}

static void conn_mutex_unlock(bcp_conn_t *conn)
{
	Thread_unlock_mutex(conn->mutex);
}

static void set_disconnected(bcp_conn_t *conn)
{
	conn_mutex_lock(conn);
	conn->connected = 0;
	conn_mutex_unlock(conn);
}

static void set_connected(bcp_conn_t *conn)
{
	conn_mutex_lock(conn);
	conn->connected = 1;
	conn_mutex_unlock(conn);
}

static void on_conn_lost(void *context, char *cause)
{
	bcp_conn_t *conn = (bcp_conn_t*)context;
	int rc;

	LOG_I("connection lost");
	if (cause) {
		LOG_I("     cause: %s", cause);
	}

	if (!conn) {
		return;
	}

	set_disconnected(conn);

	conn_mutex_lock(conn);
	conn->opts.keepAliveInterval = 20;
	conn->opts.cleansession = 1;
	conn_mutex_unlock(conn);

	LOG_I("reconnecting");
	if (MQTTASYNC_SUCCESS != (rc = MQTTAsync_connect(conn->client, &conn->opts))) {
		LOG_W("failed to start reconnect, return code %d", rc);
	}
}

static void on_disconnect(void* context, MQTTAsync_successData* response)
{
	bcp_conn_t *conn = (bcp_conn_t *)context;

	LOG_I("successful disconnection");
	
	if (!conn) {
		return;
	}
	set_disconnected(conn);
	if (conn->cbs->on_disconnected) {
		conn->cbs->on_disconnected(conn->callback_context);
	}
}

static void on_connect_failed(void* context, MQTTAsync_failureData* response)
{
	bcp_conn_t *conn = (bcp_conn_t *)context;

	if (!conn) {
		return;
	}

	LOG_I("%s disconnected from %s, rc=%d", 
		conn->clientid, conn->address, response ? response->code : 0);

	set_disconnected(conn);
	if (conn->cbs->on_disconnected) {
		conn->cbs->on_disconnected(conn->callback_context);
	}
}

static void subscribe_topics(bcp_conn_t *conn)
{
	ListElement *current = NULL;

	LOG_I("resubscribe topics");

	while (ListNextElement(&conn->topics, &current) != NULL) {
		if (current->content) {
			LOG_I("\ttopic: %s", (char*)current->content);
			bcp_conn_subscribe(conn, (const char*)current->content);
		}
	}
}

static void on_connected(void* context, MQTTAsync_successData* response)
{
	bcp_conn_t *conn = (bcp_conn_t *)context;

	if (!conn) {
		return;
	}

	LOG_I("%s connected to %s", conn->clientid, conn->address);

	set_connected(conn);
	if (conn->cbs->on_connected) {
		conn->cbs->on_connected(conn->callback_context);
	}

	conn_mutex_lock(conn);
	subscribe_topics(conn);
	conn_mutex_unlock(conn);
}

static int on_message_arrived(void* context, char* topic_name, int topic_len, MQTTAsync_message* message)
{
	bcp_conn_t *conn = (bcp_conn_t *)context;
	bcp_packet_t *p;

	if (!conn) {
		MQTTAsync_freeMessage(&message);
		MQTTAsync_free(topic_name);
		return 1;
	}

	/* TODO: packet recombine */

	if (bcp_packet_unserialize((u8*)message->payload, (u32)message->payloadlen, &p) < 0) {
		MQTTAsync_freeMessage(&message);
		MQTTAsync_free(topic_name);
		return 1;
	}

	/* call application register callback */
	if (conn->cbs->on_packet_arrived) {
		conn->cbs->on_packet_arrived(conn->callback_context, p);
	}

	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topic_name);
	return 1;
}

static void on_send(void *context, MQTTAsync_successData *response)
{
	bcp_conn_t *conn = (bcp_conn_t *)context;
	LOG_I("message with token value %d delivery confirmed", response->token);
}

static void on_message_delivered(void* context, MQTTAsync_token token)
{
	bcp_conn_t *conn = (bcp_conn_t *)context;

	if (!conn) {
		return;
	}

	LOG_I("message delivered, token = %d", token);
	if (conn->cbs->on_packet_delivered) {
		conn->cbs->on_packet_delivered(conn->callback_context, (int)token);
	}
}

static void on_subscribe(void* context, MQTTAsync_successData* response)
{
	LOG_I("subscribe succeeded");
}

static void on_subscribe_failure(void* context, MQTTAsync_failureData* response)
{
	LOG_W("subscribe failed, rc=%d", response ? response->code : 0);
}

static void on_unsubscribe(void* context, MQTTAsync_successData* response)
{
	LOG_I("unsubscribe succeeded");
}

static void on_unsubscribe_failure(void* context, MQTTAsync_failureData* response)
{
	LOG_W("unsubscribe failed, rc=%d", response ? response->code : 0);
}

static void set_def_opts(bcp_conn_t *conn)
{
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;

	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;

	conn->qos = DEF_QOS;
	memcpy(&conn->opts, &conn_opts, sizeof(MQTTAsync_connectOptions));
}

/*
 * create bcp connection
 * params 
 *   address tcp://127.0.0.0:1883
 *   clientid idenfiy from client
 * return
 *   return handle ptr or NULL
 */
void *bcp_conn_create(const char *address, const char *clientid)
{
	bcp_conn_t *conn;

	conn = (bcp_conn_t *)malloc(sizeof(*conn));
	if (!conn) {
		return NULL;
	}

	memset(conn, 0, sizeof(*conn));
	conn->address = my_strdup(address);
	conn->clientid = my_strdup(clientid);

	conn->mutex = Thread_create_mutex();
	if (!conn->mutex) {
		free(conn);
		return NULL;
	}

	if (MQTTASYNC_SUCCESS != MQTTAsync_create(&conn->client, 
		address, clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL)) {
		free(conn);
		return NULL;
	}
	if (MQTTASYNC_SUCCESS != MQTTAsync_setCallbacks(conn->client, conn, 
		on_conn_lost, on_message_arrived, on_message_delivered)) {
		MQTTAsync_destroy(&conn->client);
		free(conn);
		return NULL;
	}

	set_def_opts(conn);
	ListZero(&conn->topics);

	return conn;
}

void bcp_conn_set_callbacks(void *hdl, bcp_conn_callbacks_t *cbs)
{
	bcp_conn_t *conn = (bcp_conn_t *)hdl;

	if (!conn) {
		return;
	}

	conn_mutex_lock(conn);
	conn->cbs = cbs;
	conn_mutex_unlock(conn);
}

void bcp_conn_set_qos(void *hdl, int qos)
{
	bcp_conn_t *conn = (bcp_conn_t *)hdl;
	if (conn) {
		conn->qos = qos;
	}
}

void bcp_conn_set_keepalive(void *hdl, int keepalive_interval)
{
	bcp_conn_t *conn = (bcp_conn_t *)hdl;
	if (conn) {
		conn->opts.keepAliveInterval = keepalive_interval;
	}
}

int bcp_conn_connect(void *hdl)
{
	bcp_conn_t *conn = (bcp_conn_t *)hdl;
	int rc;

	if (!conn) {
		return -1;
	}

	conn->opts.onSuccess = on_connected;
	conn->opts.onFailure = on_connect_failed;
	conn->opts.context = conn;

	if (MQTTASYNC_SUCCESS != (rc = MQTTAsync_connect(conn->client, &conn->opts))) {
		LOG_W("bcp_conn_create failed to start connect, return code %d", rc);
		return -1;
	}

	return 0;
}

int bcp_conn_isconnected(void *hdl)
{
	bcp_conn_t *conn = (bcp_conn_t *)hdl;
	if (!conn) {
		return 0;
	}
	return MQTTAsync_isConnected(conn->client);
}

int bcp_conn_disconnect(void *hdl)
{
	bcp_conn_t *conn = (bcp_conn_t *)hdl;
	int rc;
	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;

	if (!conn) {
		return -1;
	}

	opts.onSuccess = on_disconnect;
	opts.context = conn;

	if (MQTTASYNC_SUCCESS != (rc = MQTTAsync_disconnect(conn->client, &opts))) {
		LOG_W("failed to start disconnect, return code %d", rc);
		return -1;
	}

	return 0;
}

static void remove_topics(List *list);

void bcp_conn_destroy(void *hdl)
{
	bcp_conn_t *conn = (bcp_conn_t*)hdl;

	if (!conn) {
		return;
	}

	conn_mutex_lock(conn);
	if (bcp_conn_isconnected(conn)) {
		bcp_conn_disconnect(conn);
		conn->connected = 0; /* force set */
	}
	conn_mutex_unlock(conn);

	if (conn->address) {
		free(conn->address);
	}
	if (conn->clientid) {
		free(conn->clientid);
	}

	remove_topics(&conn->topics);

	MQTTAsync_destroy(&conn->client);
	Thread_destroy_mutex(&conn->mutex);

	free(conn);
}

static void *find_topic(List *list, const char *topic)
{
	ListElement *e = ListFind(list, (void*)topic);

	if (e) {
		return e->content;
	} else {
		return NULL;
	}
}

static void append_topic(List *list, const char *topic)
{
	char *s;

	if (!find_topic(list, topic)) {
		s = my_strdup(topic);
		ListAppend(list, s, strlen(s));
	}
}

static void remove_topic(List *list, const char *topic)
{
	void *c;

	if ((c = find_topic(list, topic))) {
		ListDetach(list, c);
		free(c);
	}
}

static void remove_topics(List *list)
{
	ListElement *current = NULL;
	void *c;

	while (ListNextElement(list, &current) != NULL) {
		c = current->content;
		free(c);
		current->content = NULL;
		ListDetach(list, c);
	}

	ListEmpty(list);
}

int bcp_conn_subscribe(void *hdl, const char *topic)
{
	bcp_conn_t *conn = (bcp_conn_t *)hdl;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc;

	if (!conn) {
		return -1;
	}

	opts.onSuccess = on_subscribe;
	opts.onFailure = on_subscribe_failure;
	opts.context = conn;

	if (MQTTASYNC_SUCCESS != (rc = MQTTAsync_subscribe(conn->client, topic, conn->qos, &opts))) {
		LOG_W("failed to start subscribe, return code %d", rc);
		return -1;
	}

	conn_mutex_lock(conn);
	append_topic(&conn->topics, topic);
	conn_mutex_unlock(conn);

	return 0;
}

int bcp_conn_unsubscribe(void *hdl, const char *topic)
{
	bcp_conn_t *conn = (bcp_conn_t *)hdl;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc;

	if (!conn) {
		return -1;
	}

	opts.onSuccess = on_unsubscribe;
	opts.onFailure = on_unsubscribe_failure;
	opts.context = conn;

	if (MQTTASYNC_SUCCESS != (rc = MQTTAsync_unsubscribe(conn->client, topic, &opts))) {
		LOG_W("failed to start unsubscribe, return code %d", rc);
		return -1;
	}

	conn_mutex_lock(conn);
	remove_topic(&conn->topics, topic);
	conn_mutex_unlock(conn);

	return 0;
}

int bcp_conn_publish_raw(void *hdl, const char *topic, const char *buf, int len)
{
	bcp_conn_t *conn = (bcp_conn_t *)hdl;
	int rc;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;

	if (!conn) {
		return -1;
	}

	opts.onSuccess = on_send;
	opts.context = conn;

	pubmsg.payload = (void*)buf;
	pubmsg.payloadlen = len;
	pubmsg.qos = conn->qos;
	pubmsg.retained = 0;

	if (MQTTASYNC_SUCCESS != (rc = MQTTAsync_sendMessage(conn->client, topic, &pubmsg, &opts))) {
		LOG_W("failed to start sendMessage, return code %d", rc);
		return -1;
	}

	return 0;
}

int bcp_conn_pulish(void *hdl, const char *topic, bcp_packet_t *p)
{
	bcp_conn_t *conn = (bcp_conn_t *)hdl;
	u8 *buf;
	u32 len;

	if (!conn) {
		return -1;
	}

	if (bcp_packet_serialize(p, &buf, &len) < 0) {
		return -1;
	}
	return bcp_conn_publish_raw(conn, topic, (const char*)buf, len);
}

