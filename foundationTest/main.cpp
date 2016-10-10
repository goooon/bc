#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if defined(WIN32)
#define SPRINTF sprintf_s
#else
#define SPRINTF sprintf
#endif

#include "../fundation/src/inc/dep.h"
#include "../fundation/src/inc/bcp_packet.h"
#include "../fundation/src/inc/bcp_comm.h"
#include "../fundation/src/inc/bcp.h"

#define ADDRESS "tcp://iot.eclipse.org:1883"
#define PUB_CLIENTID "BCP_CLIENT_PUB"
#define SUB_CLIENTID "BCP_CLIENT_SUB"

#define TOPIC "/beecloud"
#define ELEMENT "ele"
#define ELEMENT2 "ele2"

static void *hdl;
static int connected = 0;

static int my_rnd(int c)
{
	srand((int)time(NULL));
	return rand() % c;
}

static void my_sleep(int milis)
{
#if defined(WIN32)
	Sleep(milis);
#else
	usleep(1000L * milis);
#endif
}

static void on_connected(void *context)
{
	LOG_I("APP:connected");
	connected = 1;
}

static void on_disconnected(void *context)
{
	LOG_I("APP:disconnected");
	connected = 0;
}

static void print_element(bcp_element_t *e)
{
	LOG_I("\t\telement: %s,%d", e->data, e->len);
}

static void bcp_element_foreach_callback(bcp_element_t *e, void *context)
{
	print_element(e);
}

static void print_message(bcp_message_t *m)
{
	LOG_I("\tmessage: seqid=%llx, appid=%d, sessid=%d, stepid=%d, ver=%d, msglen=%d", 
		m->hdr.sequence_id, m->hdr.id, m->hdr.session_id,
		m->hdr.step_id, m->hdr.version, m->hdr.message_len);
}

static void bcp_message_foreach_callback(bcp_message_t *m, void *context)
{
	print_message(m);
	bcp_elements_foreach(m, bcp_element_foreach_callback, NULL);
}

static void parse_packet(bcp_packet_t *p)
{
	bcp_message_t *m = NULL;
	bcp_element_t *e = NULL;

	//bcp_messages_foreach(p, bcp_message_foreach_callback, NULL);

	while ((m = bcp_next_message(p, m)) != NULL) {
		print_message(m);
		e = NULL;
		while ((e = bcp_next_element(m, e)) != NULL) {
			print_element(e);
		}
	}
}

static void print_packet(bcp_packet_t *p)
{
	LOG_I("APP:package arrived, v=%d, plen=%d, crc=0x%x", 
		p->hdr.version, p->hdr.packet_len, p->end.crc32);
}

static void on_packet_arrived(void *context, bcp_packet_t *p)
{
	print_packet(p);
	parse_packet(p);
	bcp_packet_destroy(p);
}

static void on_packet_delivered(void *context, int token)
{
	LOG_I("APP:package delivered, %d", token);
}

static bcp_conn_callbacks_t cbs = {
	on_connected,
	on_disconnected,
	on_packet_arrived,
	on_packet_delivered
};

static void create_elements(bcp_message_t *m, int count)
{
	int i;
	char buf[20];
	bcp_element_t *e;

	for (i = 0; i < count; i++) {
		SPRINTF(buf, "ele%d", i);
		e = bcp_element_create((u8*)buf, strlen(buf) + 1);
		bcp_element_append(m, e);
		if (i > 0 && i % 4 == 0) {
			LOG_I("deleteing %d", i);
			bcp_element_destroy(e);
		}
	}
}

static void create_messages(bcp_packet_t *p, int count)
{
	int i;
	bcp_message_t *m;

	for (i = 0; i < count; i++) {
		m = bcp_message_create(i, i + 1, i + 2, i + 3);
		bcp_message_append(p, m);
		create_elements(m, my_rnd(5) + i);
		if (i > 0 && i % 3 == 0) {
			LOG_I("deleteing %d", i);
			bcp_message_destroy(m);
		}
	}
}

static void publish_packet(void)
{
	bcp_packet_t *p;
	u8 *data;
	u32 len;

	p = bcp_packet_create(2);
	if (!p) {
		return;
	}

	create_messages(p, my_rnd(20));

	if (bcp_packet_serialize(p, &data, &len) >= 0) {
		bcp_conn_pulish(hdl, TOPIC, p);
	}

	bcp_packet_destroy(p);
}

static void publish(void)
{
	hdl = bcp_conn_create(ADDRESS, PUB_CLIENTID);
	if (!hdl) {
		return;
	}

	bcp_conn_set_callbacks(hdl, &cbs);
	bcp_conn_set_qos(hdl, 1);
	bcp_conn_set_keepalive(hdl, 20);

	bcp_conn_connect(hdl);
	while (!connected) {
		my_sleep(10);
	}

	while (connected) {
		publish_packet();
		my_sleep(1000);
	}
}

static void subscribe(void)
{
	hdl = bcp_conn_create(ADDRESS, SUB_CLIENTID);
	if (!hdl) {
		return;
	}

	bcp_conn_set_callbacks(hdl, &cbs);
	bcp_conn_set_qos(hdl, 1);
	bcp_conn_set_keepalive(hdl, 20);

	bcp_conn_connect(hdl);
	while (!connected) {
		my_sleep(10);
	}

	bcp_conn_subscribe(hdl, TOPIC);
	//wait topic message

	while (connected) {
		my_sleep(10);
	}
}

int main(int argc, char **argv)
{
	int ispub;
	
	if (argc < 2) {
		printf("usage %s {0|1}", argv[0]);
		return -1;
	}

	bcp_init();

	ispub = atoi(argv[1]);
	if (ispub) {
		publish();
	} else {
		subscribe();
	}

	bcp_uninit();

	return 0;
}
