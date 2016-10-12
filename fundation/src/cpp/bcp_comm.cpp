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
	MQTTAsync client;
	MQTTAsync_connectOptions opts;

	int qos;
	void *callback_context;
	bcp_conn_callbacks_t *cbs;

	/* subscribe topics */
	List topics;
	int locked;
} bcp_conn_t;

typedef struct bcp_pub_context_s
{
	char *topic;
	bcp_conn_t *conn;
	void *context;
} bcp_pub_context_t;

typedef struct bcp_sub_context_s
{
	char *topic;
	bcp_conn_t *conn;
	void *context;
} bcp_sub_context_t;

#define DEF_QOS	1
#define DEF_KEEPALIVE 30

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

static void my_sleep(int milis)
{
#if defined(WIN32)
	Sleep(milis);
#else
	usleep(1000L * milis);
#endif
}

static List conns;
static mutex_type conns_mutex = NULL;

void bcp_conn_init(void)
{
	ListZero(&conns);
	conns_mutex = Thread_create_mutex();
	if (!conns_mutex) {
		LOG_W("bcp_conn_init create mutex failed");
	}
}

void bcp_conn_uninit(void)
{
	Thread_lock_mutex(conns_mutex);
	if (conns.count) {
		Thread_unlock_mutex(conns_mutex);
		LOG_W("bcp_conn_uninit has %d connection need destroy.", 
			conns.count);
		return;
	}
	ListEmpty(&conns);
	Thread_unlock_mutex(conns_mutex);

	Thread_destroy_mutex(conns_mutex);
	conns_mutex = NULL;
}

static void conn_lock(bcp_conn_t *conn)
{
	Thread_lock_mutex(conn->mutex);
}

static void conn_unlock(bcp_conn_t *conn)
{
	Thread_unlock_mutex(conn->mutex);
}

static void conns_list_lock(void)
{
	Thread_lock_mutex(conns_mutex);
}

static void conns_list_unlock(void)
{
	Thread_unlock_mutex(conns_mutex);
}

static bcp_conn_t *find_connect(bcp_conn_t *conn)
{
	ListElement *e;
	bcp_conn_t *c = NULL;

	e = ListFind(&conns, (void*)conn);
	if (e) {
		c = (bcp_conn_t*)e->content;
	}

	return c;
}

static void insert_connect(bcp_conn_t *conn)
{
	if (!find_connect(conn)) {
		ListAppend(&conns, (void*)conn, sizeof(*conn));
	}
}

static void remove_connect(bcp_conn_t *conn)
{
	if (find_connect(conn)) {
		ListDetach(&conns, (void*)conn);
	}
}

static bcp_conn_t *lock_connect(bcp_conn_t *conn)
{
	bcp_conn_t *c;

	conns_list_lock();
	c = find_connect(conn);
	if (c) {
		conn_lock(c);
		++c->locked;
		conn_unlock(c);
	}
	conns_list_unlock();

	return c;
}

static void unlock_connect(bcp_conn_t *conn)
{
	if (conn) {
		conn_lock(conn);
		if (conn->locked > 0) {
			--conn->locked;
		}
		conn_unlock(conn);
	}
}

static bcp_sub_context_t *create_sub_context(bcp_conn_t *conn,
	const char *topic, void *context)
{
	bcp_sub_context_t *c;

	if (!conn || !topic) {
		return NULL;
	}

	c = (bcp_sub_context_t*)malloc(sizeof(*c));
	if (c) {
		c->conn = conn;
		c->context = context;
		c->topic = my_strdup(topic);
	}

	return c;
}

static void free_sub_context(bcp_sub_context_t *c)
{
	if (c) {
		free(c->topic);
		free(c);
	}
}

static bcp_pub_context_t *create_pub_context(bcp_conn_t *conn,
	const char *topic, void *context)
{
	bcp_pub_context_t *c;

	if (!conn || !topic) {
		return NULL;
	}

	c = (bcp_pub_context_t*)malloc(sizeof(*c));
	if (c) {
		c->conn = conn;
		c->context = context;
		c->topic = my_strdup(topic);
	}

	return c;
}

static void free_pub_context(bcp_pub_context_t *c)
{
	if (c) {
		free(c->topic);
		free(c);
	}
}

static void on_conn_lost(void *context, char *cause)
{
	bcp_conn_t *conn = (bcp_conn_t*)context;

	conn = lock_connect(conn);

	LOG_I("connection lost");
	if (cause) {
		LOG_I("\tcause: %s", cause);
	}

	if (!conn) {
		return;
	}

	LOG_I("reconnecting");
	bcp_conn_connect(conn);

	unlock_connect(conn);
}

static void subscribe_topics(bcp_conn_t *conn);
static void on_connected(void* context, MQTTAsync_successData* response)
{
	bcp_conn_t *conn = (bcp_conn_t *)context;

	conn = lock_connect(conn);

	if (!conn) {
		return;
	}

	LOG_I("%s connected to %s", conn->clientid, conn->address);

	if (conn->cbs->on_connected) {
		conn->cbs->on_connected(conn->callback_context);
	}

	subscribe_topics(conn);
	unlock_connect(conn);
}

static void on_disconnect(void* context, MQTTAsync_successData* response)
{
	bcp_conn_t *conn = (bcp_conn_t *)context;

	conn = lock_connect(conn);

	if (!conn) {
		return;
	}

	LOG_I("%s disconnected from %s", 
		conn->clientid, conn->address);

	if (conn->cbs->on_disconnected) {
		conn->cbs->on_disconnected(conn->callback_context);
	}

	unlock_connect(conn);
}

static void on_connect_failed(void* context, MQTTAsync_failureData* response)
{
	bcp_conn_t *conn = (bcp_conn_t *)context;
	
	conn = lock_connect(conn);

	if (!conn) {
		return;
	}

	LOG_I("%s connection failure from %s, rc=%d", 
		conn->clientid, conn->address, response ? response->code : 0);

	if (conn->cbs->on_connect_failed) {
		conn->cbs->on_connect_failed(conn->callback_context);
	}

	unlock_connect(conn);
}

static int on_message_arrived(void *context, 
	char *topic_name, int topic_len, MQTTAsync_message *message)
{
	bcp_conn_t *conn = (bcp_conn_t *)context;
	bcp_packet_t *p;

	conn = lock_connect(conn);

	if (!conn) {
		MQTTAsync_freeMessage(&message);
		MQTTAsync_free(topic_name);
		return 1;
	}

	/* TODO: packet recombine */

	if (bcp_packet_unserialize((u8*)message->payload, (u32)message->payloadlen, &p) < 0) {
		MQTTAsync_freeMessage(&message);
		MQTTAsync_free(topic_name);
		unlock_connect(conn);
		return 1;
	}

	/* call application register callback */
	if (conn->cbs->on_packet_arrived) {
		conn->cbs->on_packet_arrived(conn->callback_context, p);
	}

	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topic_name);
	unlock_connect(conn);
	return 1;
}

static void on_message_delivered(void *context, MQTTAsync_token token)
{
	bcp_conn_t *conn = (bcp_conn_t *)context;

	conn = lock_connect(conn);

	if (!conn) {
		return;
	}

	LOG_I("delivered to server, token = %d", token);
	unlock_connect(conn);
}

static void on_publish(void *context, MQTTAsync_successData *response)
{
	bcp_pub_context_t *c = (bcp_pub_context_t*)context;
	bcp_conn_t *conn;

	if (!c) {
		return;
	}

	LOG_I("published, %d:%s", response ? response->token : 0, 
		c->topic);

	conn = lock_connect(c->conn);

	if (!conn) {
		free_pub_context(c);
		return;
	}

	if (conn->cbs->on_packet_delivered) {
		conn->cbs->on_packet_delivered(c->context, response ? response->token : 0);
	}
	free_pub_context(c);
	unlock_connect(conn);
}

static void on_publish_failure(void *context, MQTTAsync_failureData* response)
{
	bcp_pub_context_t *c = (bcp_pub_context_t*)context;
	bcp_conn_t *conn;

	if (!c) {
		return;
	}

	LOG_W("publish %d:%s failed, rc=%d", 
		response ? response->token : 0, 
		c->topic, response ? response->code : 0);

	conn = lock_connect(c->conn);

	if (!conn) {
		free_pub_context(c);
		return;
	}
	if (conn->cbs->on_packet_deliver_failed) {
		conn->cbs->on_packet_deliver_failed(c->context, response ? response->token : 0);
	}
	free_pub_context(c);
	unlock_connect(conn);
}

static void on_subscribe(void* context, MQTTAsync_successData* response)
{
	bcp_sub_context_t *c = (bcp_sub_context_t*)context;
	bcp_conn_t *conn;

	if (!c) {
		return;
	}
	LOG_I("subscribe, topic: %s", c->topic);

	conn = lock_connect(c->conn);

	if (!conn) {
		free_sub_context(c);
		return;
	}
	if (conn->cbs->on_subscribe) {
		conn->cbs->on_subscribe(c->context, my_strdup(c->topic));
	}
	free_sub_context(c);
	unlock_connect(conn);
}

static void on_subscribe_failure(void* context, MQTTAsync_failureData* response)
{
	bcp_sub_context_t *c = (bcp_sub_context_t*)context;
	bcp_conn_t *conn;
	
	if (!c) {
		return;
	}
	LOG_W("subscribe failed, topic: %s, rc: %d", 
		c->topic, response ? response->code : 0);

	conn = lock_connect(c->conn);

	if (!conn) {
		free_sub_context(c);
		return;
	}
	if (conn->cbs->on_subscribe_failed) {
		conn->cbs->on_subscribe_failed(c->context, my_strdup(c->topic));
	}
	free_sub_context(c);
	unlock_connect(conn);
}

static void on_unsubscribe(void* context, MQTTAsync_successData* response)
{
	bcp_sub_context_t *c = (bcp_sub_context_t*)context;
	bcp_conn_t *conn;

	if (!c) {
		return;
	}
	LOG_I("unsubscribe succeeded, topic: %s", c->topic);

	conn = lock_connect(c->conn);

	if (!conn) {
		free_sub_context(c);
		return;
	}
	if (conn->cbs->on_unsubscribe) {
		conn->cbs->on_unsubscribe(c->context, my_strdup(c->topic));
	}
	free_sub_context(c);
	unlock_connect(conn);
}

static void on_unsubscribe_failure(void* context, MQTTAsync_failureData* response)
{
	bcp_sub_context_t *c = (bcp_sub_context_t*)context;
	bcp_conn_t *conn;

	if (!c) {
		return;
	}

	LOG_W("unsubscribe failed, topic: %s, rc: %d", 
		c->topic, response ? response->code : 0);

	conn = lock_connect(c->conn);

	if (!conn) {
		free_sub_context(c);
		return;
	}
	if (conn->cbs->on_unsubscribe_failed) {
		conn->cbs->on_unsubscribe_failed(c->context, my_strdup(c->topic));
	}
	free_sub_context(c);
	unlock_connect(conn);
}

static void set_def_opts(bcp_conn_t *conn)
{
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;

	conn_opts.keepAliveInterval = DEF_KEEPALIVE;
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
	conn->locked = 0;

	conns_list_lock();
	insert_connect(conn);
	conns_list_unlock();

	return conn;
}

void bcp_conn_set_callbacks(void *hdl, bcp_conn_callbacks_t *cbs, void *context)
{
	bcp_conn_t *conn = (bcp_conn_t *)hdl;

	if (!conn) {
		return;
	}

	conn_lock(conn);
	conn->cbs = cbs;
	conn->callback_context = context;
	conn_unlock(conn);
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
static void unsubcribe_topics(bcp_conn_t *conn);

void bcp_conn_destroy(void *hdl)
{
	bcp_conn_t *conn = (bcp_conn_t*)hdl;

	if (!conn) {
		return;
	}

	conn_lock(conn);
	if (bcp_conn_isconnected(conn)) {
		bcp_conn_disconnect(conn);
	}

	while (conn->locked > 0) {
		conn_unlock(conn);
		my_sleep(10);
		conn_lock(conn);
	}

	unsubcribe_topics(conn);
	remove_topics(&conn->topics);

	if (conn->address) {
		free(conn->address);
		conn->clientid =  NULL;
	}
	if (conn->clientid) {
		free(conn->clientid);
		conn->clientid =  NULL;
	}

	conn->cbs = NULL;
	conn_unlock(conn);

	conns_list_lock();
	remove_connect(conn);
	conns_list_unlock();

	MQTTAsync_destroy(&conn->client);
	Thread_destroy_mutex(conn->mutex);

	free(conn);
}

static void *find_topic(List *list, const char *topic)
{
	ListElement *e;
	void *c = NULL;

	e = ListFind(list, (void*)topic);
	if (e) {
		c = e->content;
	}

	return c;
}

static void insert_topic(List *list, const char *topic)
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
		current = NULL;
	}

	ListEmpty(list);
}

static void subscribe_topics(bcp_conn_t *conn)
{
	ListElement *current = NULL;

	LOG_I("resubscribe topics");

	while (ListNextElement(&conn->topics, &current) != NULL) {
		if (current->content) {
			LOG_I("\ttopic: %s", (char*)current->content);
			bcp_conn_subscribe(conn, (const char*)current->content, NULL);
		}
	}
}

static void unsubcribe_topics(bcp_conn_t *conn)
{
	ListElement *current = NULL;
	void *c;

	while (ListNextElement(&conn->topics, &current) != NULL) {
		c = current->content;
		if (c) {
			bcp_conn_unsubscribe(conn, (const char*)c, NULL);
		}
	}
}

int bcp_conn_subscribe(void *hdl, const char *topic,
	void *context)
{
	bcp_conn_t *conn = (bcp_conn_t *)hdl;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc;

	if (!conn) {
		return -1;
	}

	opts.onSuccess = on_subscribe;
	opts.onFailure = on_subscribe_failure;
	opts.context = create_sub_context(conn, topic, context);

	if (MQTTASYNC_SUCCESS != (rc = MQTTAsync_subscribe(conn->client, topic, conn->qos, &opts))) {
		LOG_W("failed to start subscribe, return code %d", rc);
		free_sub_context((bcp_sub_context_t*)opts.context);
		return -1;
	}

	conn_lock(conn);
	insert_topic(&conn->topics, topic);
	conn_unlock(conn);

	return 0;
}

int bcp_conn_unsubscribe(void *hdl, const char *topic, 
	void *context)
{
	bcp_conn_t *conn = (bcp_conn_t *)hdl;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc;

	if (!conn) {
		return -1;
	}

	opts.onSuccess = on_unsubscribe;
	opts.onFailure = on_unsubscribe_failure;
	opts.context = create_sub_context(conn, topic, context);

	if (MQTTASYNC_SUCCESS != (rc = MQTTAsync_unsubscribe(conn->client, topic, &opts))) {
		LOG_W("failed to start unsubscribe, return code %d", rc);
		free_sub_context((bcp_sub_context_t*)opts.context);
		return -1;
	}

	conn_lock(conn);
	remove_topic(&conn->topics, topic);
	conn_unlock(conn);

	return 0;
}

int bcp_conn_publish_raw(void *hdl, const char *buf, int len,
	const char *topic, void *context)
{
	bcp_conn_t *conn = (bcp_conn_t *)hdl;
	int rc;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;

	if (!conn) {
		return -1;
	}

	opts.onSuccess = on_publish;
	opts.onFailure = on_publish_failure;
	opts.context = create_pub_context(conn, topic, context);

	pubmsg.payload = (void*)buf;
	pubmsg.payloadlen = len;
	pubmsg.qos = conn->qos;
	pubmsg.retained = 0;

	if (MQTTASYNC_SUCCESS != (rc = MQTTAsync_sendMessage(conn->client, topic, &pubmsg, &opts))) {
		LOG_W("failed to start sendMessage, return code %d", rc);
		free_pub_context((bcp_pub_context_t*)opts.context);
		return -1;
	}

	return 0;
}

int bcp_conn_pulish(void *hdl, bcp_packet_t *p, const char *topic, void *context)
{
	int ret;
	bcp_conn_t *conn = (bcp_conn_t *)hdl;
	u8 *buf;
	u32 len;

	if (!conn) {
		return -1;
	}

	if (bcp_packet_serialize(p, &buf, &len) < 0) {
		return -1;
	}

	ret = bcp_conn_publish_raw(conn, (const char*)buf, len, topic, context);
	free(buf);

	return ret;
}

