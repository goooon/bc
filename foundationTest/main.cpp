#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#if defined(WIN32) || defined(WIN64)
#else
#include <unistd.h>
#define WINAPI
#endif

#if defined(WIN32)
#define SPRINTF sprintf_s
#else
#define SPRINTF sprintf
#endif

#include "../fundation/src/inc/util/Thread.h"
#include "../fundation/src/inc/util/LinkedList.h"

#include "../fundation/src/inc/dep.h"
#include "../fundation/src/inc/bcp_packet.h"
#include "../fundation/src/inc/bcp_comm.h"
#include "../fundation/src/inc/bcp_serial.h"
#include "../fundation/src/inc/bcp_nmea.h"
#include "../fundation/src/inc/bcp.h"

#include "../fundation/src/inc/binary_formater.h"

//#define ADDRESS "tcp://139.219.238.66:1883"
#define ADDRESS "tcp://iot.eclipse.org:1883"
#define PUB_CLIENTID "BCP_CLIENT_PUB"
#define SUB_CLIENTID "BCP_CLIENT_SUB"

#define TOPIC_SUB "/beecloud"
//#define TOPIC_SUB "mqtt/notify/15218"
#define TOPIC_PUB "/beecloud"

#define ELEMENT "ele"
#define ELEMENT2 "ele2"
#define ELEMENT_ONE_MSG "ele_one_msg"

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
}

static void on_disconnected(void *context)
{
	LOG_I("APP:disconnected");
}

static void print_element(bcp_element_t *e)
{
	char *p = (char*)malloc(e->len + 1);
	memset(p, 0, e->len + 1);
	memcpy(p, e->data, e->len);
	LOG_I("\t\telement: %s,%d", p, e->len);
}

static void bcp_element_foreach_callback(bcp_element_t *e, void *context)
{
	print_element(e);
}

static void print_message(bcp_message_t *m)
{
	LOG_I("\tmessage: seqid=%llx, appid=%d, stepid=%d, ver=%d, msglen=%d", 
		m->hdr.sequence_id, m->hdr.id, m->hdr.step_id, 
		m->hdr.version, m->hdr.message_len);
}

static void bcp_message_foreach_callback(bcp_message_t *m, void *context)
{
	print_message(m);
	bcp_elements_foreach(m, bcp_element_foreach_callback, NULL);
}

static void print_packet(bcp_packet_t *p)
{
	LOG_I("APP:package arrived, v=%d, plen=%d, crc=0x%x",
		p->hdr.version, p->hdr.packet_len, p->end.crc32);
}

static void parse_packet(bcp_packet_t *p)
{
	bcp_message_t *m = NULL;
	bcp_element_t *e = NULL;
	
	print_packet(p);
	//bcp_messages_foreach(p, bcp_message_foreach_callback, NULL);

	while ((m = bcp_next_message(p, m)) != NULL) {
		print_message(m);
		e = NULL;
		while ((e = bcp_next_element(m, e)) != NULL) {
			print_element(e);
		}
	}
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

static void on_connect_failed(void *context)
{
	LOG_I("APP:connection failed");
}
static void on_packet_deliver_failed(void *context, int token)
{
	LOG_I("APP:packet deliver failed");
}

static void on_subscribe(void *context, char *topic)
{
	LOG_I("APP:subscribe topic: %s", topic);
	free(topic);
}

static void on_subscribe_failed(void *context, char *topic)
{
	LOG_I("APP:unsubscribe failed, topic: %s", topic);
	free(topic);
}

static void on_unsubscribe(void *context, char *topic)
{
	LOG_I("APP:subscribe topic: %s", topic);
	free(topic);
}

static void on_unsubscribe_failed(void *context, char *topic)
{
	LOG_I("APP:unsubscribe failed, topic: %s", topic);
	free(topic);
}

static bcp_conn_callbacks_t cbs = {
	on_connected,
	on_connect_failed,
	on_disconnected,
	
	on_packet_arrived,
	
	on_packet_delivered,
	on_packet_deliver_failed,

	on_subscribe,
	on_subscribe_failed,
	
	on_unsubscribe,
	on_unsubscribe_failed,
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
		m = bcp_message_create((u16)i, (u8)i + 1, bcp_next_seq_id());
		bcp_message_append(p, m);
		create_elements(m, my_rnd(3) + i);
		if (i > 0 && i % 3 == 0) {
			LOG_I("deleteing %d", i);
			bcp_message_destroy(m);
		}
	}
}

static void publish_one_message(const char *topic, void *hdl)
{
	bcp_packet_t *p, *pu;
	u8 *data;
	u32 len;

	p = bcp_create_one_message((u16)2, (u8)5, bcp_next_seq_id(), 
		(u8*)ELEMENT_ONE_MSG, sizeof(ELEMENT_ONE_MSG));

	if (bcp_packet_serialize(p, &data, &len) >= 0) {
		bcp_conn_pulish(hdl, p, topic, NULL);
		/*
		if (bcp_packet_unserialize(data, len, &pu) >= 0) {
			parse_packet(pu);
			bcp_packet_destroy(pu);
		}*/
		free(data);
	}

	bcp_packet_destroy(p);
}

static void publish_packet(const char *topic, void *hdl)
{
	bcp_packet_t *p;
	u8 *data;
	u32 len;

	p = bcp_packet_create();
	if (!p) {
		return;
	}

	create_messages(p, my_rnd(10));
	if (0) {
		bcp_conn_pulish(hdl, p, topic, NULL);
	} else {
		if (bcp_packet_serialize(p, &data, &len) >= 0) {
			bcp_conn_publish_raw(hdl, (const char*)data, len, topic, NULL);
			free(data);
		}
	}

	bcp_packet_destroy(p);
}

static void test_reconnect(void *hdl)
{
	LOG_I("REQ disconnection");
	bcp_conn_disconnect(hdl);
	while (bcp_conn_isconnected(hdl)) {/* wait disconnected */
		printf("waiting disconnected\n");
		my_sleep(1000);
	}

	LOG_I("REQ reconnection");
	bcp_conn_connect(hdl);
	while (!bcp_conn_isconnected(hdl)) { /* wait connected */
		printf("waiting connected\n");
		my_sleep(1000);
	}
	my_sleep(5000);
}

static void publish(const char *clientid, const char *topic)
{
	void *hdl;
	int times = 20;

	hdl = bcp_conn_create(ADDRESS, clientid);
	if (!hdl) {
		return;
	}

	bcp_conn_set_callbacks(hdl, &cbs, NULL);
	bcp_conn_set_qos(hdl, 1);
	bcp_conn_set_keepalive(hdl, 20);

	bcp_conn_connect(hdl);
	while (!bcp_conn_isconnected(hdl)) {
		printf("waiting connected\n");
		my_sleep(1000);
	}

	while (bcp_conn_isconnected(hdl) /*&& times-- > 0*/) {
		bcp_conn_connect(hdl);
		publish_packet(topic, hdl);
		//publish_one_message(topic, hdl);
		my_sleep(1000);
	}

	test_reconnect(hdl);

	bcp_conn_disconnect(hdl);
	bcp_conn_destroy(hdl);
	hdl = NULL;
	printf("publish %s exit.\n", topic);
}

static void subscribe(const char *clientid, const char *topic)
{
	void *hdl;
	int times = 10;

	hdl = bcp_conn_create(ADDRESS, clientid);
	if (!hdl) {
		return;
	}

	bcp_conn_set_callbacks(hdl, &cbs, NULL);
	bcp_conn_set_qos(hdl, 1);
	bcp_conn_set_keepalive(hdl, 20);

	bcp_conn_connect(hdl);
	while (!bcp_conn_isconnected(hdl)) {
		printf("waiting connected\n");
		my_sleep(1000);
	}

	bcp_conn_subscribe(hdl, topic, NULL);
	//wait topic message

	while (bcp_conn_isconnected(hdl) /*&& times-- > 0*/) {
		my_sleep(1000);
	}

	test_reconnect(hdl);

	bcp_conn_disconnect(hdl);
	bcp_conn_destroy(hdl);
	hdl = NULL;
	printf("subscribe %s exit.\n", topic);
}

static thread_return_type WINAPI publish_thread(void *arg)
{
	int i = (int)arg;
	char clientid[20];
	char topic[20];
	
	SPRINTF(clientid, "%s%d", PUB_CLIENTID, i);
	SPRINTF(topic, "%s", TOPIC_PUB);
	//SPRINTF(topic, "%s%d", TOPIC_PUB, i);
	printf("publish clientid: %s, topic: %s\n", clientid, topic);
	publish(clientid, topic);

	return NULL;
}

static void publishs(void)
{
	int threads = 1;

	while (threads-- > 0) {
		Thread_start(publish_thread, (void*)threads);
		my_sleep(3000);
	}
}

static thread_return_type WINAPI subscribe_thread(void *arg)
{
	int i = (int)arg;
	char clientid[20];
	char topic[20];

	SPRINTF(clientid, "%s%d", SUB_CLIENTID, i);
	SPRINTF(topic, "%s", TOPIC_SUB);
	//SPRINTF(topic, "%s%d", TOPIC_SUB, i);
	printf("subscribe clientid: %s, topic: %s\n", clientid, topic);
	subscribe(clientid, topic);

	return NULL;
}

static void subscribes(void)
{
	int threads = 1;

	while (threads-- > 0) {
		Thread_start(subscribe_thread, (void*)threads);
		my_sleep(3000);
	}
}

static void test_bcp_api(void)
{
	while (1) {
		char *pbuf;
		bcp_packet_t *p;
		bcp_message_t *m;
		bcp_element_t *e;
		p = bcp_packet_create();
			m = bcp_message_create(1, 1, 1);
			bcp_message_append(p, m);
				e = bcp_element_create((u8*)(pbuf = (char*)malloc(1)), 1);
				bcp_element_append(m, e);
				bcp_element_destroy(e);
				free(pbuf);
			bcp_message_destroy(m);
		bcp_packet_destroy(p);
	}

	while (1) {
		List l;
		char buf[1] = { 0, };

		ListZero(&l);
		ListAppend(&l, buf, 1);
		ListDetach(&l, buf);

		ListEmpty(&l);
	}
}

static void test_binary_formater(void)
{
	u32 i;
	u8 u8data2, u8data = 0xf1;
	u16 u16data2, u16data = 0xf2f3;
	u32 u24data2, u24data = 0xf4f5f6UL;
	u32 u32data2, u32data = 0xf7f8f9UL;
	u64 u64data2, u64data = 0xfafbfcfdfeefdfcfULL;
	u8 *recv_bytes, bytes[5] = {6, 7, 8, 9, 10};
	u32 recv_bytes_len;
	u8 *rs, *s = (u8*)"hello, world!";
	
	bf_t *h = bf_create_encoder();
	if (!h) {
		LOG_W("create bf encoder failed");
		return;
	}

	if (bf_put_u8(h, u8data) < 0) {
		LOG_W("put u8 failed");
	} else {
		LOG_I("put u8 = 0x%x", u8data);
	}
	if (bf_put_u16(h, u16data) < 0) {
		LOG_W("put u16 failed");
	} else {
		LOG_I("put u16 = 0x%x", u16data);
	}
	if (bf_put_u24(h, u24data) < 0) {
		LOG_W("put u24 failed");
	} else {
		LOG_I("put u24 = 0x%x", u24data);
	}
	if (bf_put_u32(h, u32data) < 0) {
		LOG_W("put u32 failed");
	} else {
		LOG_I("put u32 = 0x%x", u32data);
	}
	if (bf_put_u64(h, u64data) < 0) {
		LOG_W("put u64 failed");
	} else {
		LOG_I("put u64 = 0x%llx", u64data);
	}
	if (bf_put_bytes(h, bytes, sizeof(bytes)) < 0) {
		LOG_W("put bytes failed");
	} else {
		LOG_I("put bytes len = %d, bytes = ", sizeof(bytes));
		for (i = 0; i < sizeof(bytes); i++) {
			printf("\t%d ", bytes[i]);
		}
		printf("\n");
	}
	if (bf_put_string(h, (const char*)s) < 0) {
		LOG_W("put string failed");
	} else {
		LOG_I("put string = %s", s);
	}

	bf_reset(h, 0);

	if (bf_read_u8(h, &u8data2) < 0) {
		LOG_W("read u8 failed");
	} else {
		LOG_I("read u8 = 0x%x", u8data2);
	}
	if (bf_read_u16(h, &u16data2) < 0) {
		LOG_W("read u16 failed");
	} else {
		LOG_I("read u16 = 0x%x", u16data2);
	}
	if (bf_read_u24(h, &u24data2) < 0) {
		LOG_W("read u24 failed");
	} else {
		LOG_I("read u24 = 0x%x", u24data2);
	}
	if (bf_read_u32(h, &u32data2) < 0) {
		LOG_W("read u32 failed");
	} else {
		LOG_I("read u32 = 0x%x", u32data2);
	}
	if (bf_read_u64(h, &u64data2) < 0) {
		LOG_W("read u64 failed");
	} else {
		LOG_I("read u64 = 0x%llx", u64data2);
	}
	if (bf_read_bytes(h, &recv_bytes, &recv_bytes_len) < 0) {
		LOG_W("read bytes failed");
	} else {
		LOG_I("read bytes, len = %d, bytes = ", recv_bytes_len);
		for (i = 0; i < recv_bytes_len; i++) {
			printf("\t%d ", recv_bytes[i]);
		}
		printf("\n");
		free(recv_bytes);
	}
	if (bf_read_string(h, (char**)&rs) < 0) {
		LOG_W("put string failed");
	} else {
		LOG_I("put string = %s", s);
		free(rs);
	}

	bf_destroy(h);
}

#if defined (_WIN32) || defined(_WIN64)
#define SERIAL_DEVNAME "COM6"
#else
#define SERIAL_DEVNAME "/dev/ttySAC2"
#endif
static void serial_test(int argc, char **argv)
{
	int r;
	int i = 0;
	char buff[128] = {0,};
	void *s;

	if (argc < 2) {
		printf("usage: %s </dev/ttySAC0>\n", argv[0]);
		return;
	}

	if (!(s = bcp_serial_open(argv[1], 9600, 8, P_NONE, 1))) {
		printf("open %s failed.\n", SERIAL_DEVNAME);
		return;
	}

	my_sleep(1000 * 4);

	sprintf(buff, "%d", i++);
	printf("write %d bytes\n", 
		bcp_serial_write(s, buff, strlen(buff)));

	while ((r = bcp_serial_read(s, buff, 10, 1000)) >= 0) {
		if (r > 0) {
			buff[r] = 0;
			printf("c=%d, r=%s\n", i, buff);
			sprintf(buff, "%d", ++i);
			bcp_serial_write(s, buff, strlen(buff));
			my_sleep(500);
		} else {
			printf("-\n");
		}
	}

	bcp_serial_close(s);
}

static void print_nmea_info(bcp_nmea_info_t *info)
{
	double db = 0;

	LOG_I("\r\n\r\n");
	LOG_I("====================================\n");
	LOG_I("utc: %4d/%02d/%02d-%02d:%02d:%02d:%04d\n",
		info->utc.year, info->utc.mon, info->utc.day,
		info->utc.hour, info->utc.min, info->utc.sec, info->utc.milisec);
	LOG_I("localtime: %4d/%02d/%02d-%02d:%02d:%02d:%04d\n",
		info->localtime.year, info->localtime.mon, info->localtime.day,
		info->localtime.hour, info->localtime.min, info->localtime.sec, info->localtime.milisec);

	LOG_I("longitude: %f %C\n", info->longitude, (info->longitude < db) ? 'W' : 'E');
	LOG_I("latitude: %f %C\n", info->latitude, (info->latitude < db) ? 'S' : 'N');

	LOG_I("elevation: %f(M)\n", info->elevation);
	LOG_I("speed: %f(kph)\n", info->speed);
	LOG_I("track: %f\n", info->track);

	LOG_I("sig: %s\n", bcp_nmea_sig_to_string(info->sig));
	LOG_I("fix: %s\n", bcp_nmea_fix_to_string(info->fix));

	LOG_I("unused satellites: %d\n", info->satellites.view_count);
	LOG_I("used satellites: %d\n", info->satellites.use_count);
	
	LOG_I("pdop: %f\n", info->pdop);
	LOG_I("hdop: %f\n", info->hdop);
	LOG_I("vdop: %f\n", info->vdop);
}

static void nmea_test(void)
{
	int r;
	char buff[2048] = {0,};
	void *s;
	void *p;
	bcp_nmea_info_t *info;

	p = bcp_nmea_create(NULL, NULL);
	if (!p) {
		LOG_I("bcp nmea create failed\n");
		return;
	}

	if (!(s = bcp_serial_open(SERIAL_DEVNAME, 9600, 8, P_NONE, 1))) {
		LOG_I("open %s failed.\n", SERIAL_DEVNAME);
		bcp_nmea_destroy(p);
		return;
	}

	while ((r = bcp_serial_read(s, buff, 1, 1000)) >= 0) {
		if (r > 0) {
			if (bcp_nmea_parse(p, buff, 1) > 0) {
				/* has new sentence */
				info = bcp_nmea_info(p);
				if (info) {
					print_nmea_info(info);
				}
			}
			memset(buff, 0, sizeof(buff));
		} else {
			printf("-\n");
		}
	}

	bcp_serial_close(s);
	bcp_nmea_destroy(p);

}

#include "../fundation/src/inc/vicp/bcp_vicp.h"
#if defined (_WIN32) || defined(_WIN64)
#define CHANNEL_SERIAL "COM7"
#else
#define CHANNEL_SERIAL "/dev/ttySAC2"
#endif

static void vicp_data_arrived(void *context, u8 *buf, u16 len)
{
	bcp_packet_t *p;
	
#if 0
	LOG_I("data arrived, len=%d\n", len);
	if (bcp_packet_unserialize(buf, len, &p) < 0) {
		LOG_E("data unserialize failed.\n");
	} else {
		parse_packet(p);
		bcp_packet_destroy(p);
	}
#endif

	free(buf);
}

static void vicp_send_cb(void *context, int result)
{
	static int index = 1;
	bcp_packet_t *p = (bcp_packet_t*)context;

#if 0
	if (p) {
		LOG_I("data send complete, ver = %d, result=%d, index=%d, %p\n", 
			p->hdr.version, result, index++, p);
		bcp_packet_destroy(p);
	} else {
		LOG_I("data send complete, result=%d, index=%d\n", result, index++);
	}
#else
	if (p) {
		bcp_packet_destroy(p);
	}
	if (result != 0) {
		LOG_I("send, result=%d, index=%d\n", result, index++);
	}
#endif
}

//#define ELE_LEN (1024 - 32 + 1024 + 1)
//#define ELE_LEN (1)
static void send_one_packet(bcp_channel_t *c, 
	const char *dev_name, int dlen)
{
	static int i = 1;
	int k;
	bcp_packet_t *p, *pu;
	u8 *data;
	u32 len;

	data = (u8*)malloc(dlen);
	if (!data) {
		LOG_E("malloc failed.\n");
		return;
	}
#if 0
	sprintf((char*)data, "%d:%s\n", i++, dev_name);
	data[ELE_LEN - 1] = '0';
#endif

	for (k = 0; k < dlen; k++) {
		data[k] = '>';
	}

	p = bcp_create_one_message((u16)2, (u8)5, bcp_next_seq_id(), 
		data, dlen);
	free(data);

	if (bcp_packet_serialize(p, &data, &len) >= 0) {
		printf("send data index=%d, size=%d, p=%p\n", i++, len, p);
		bcp_vicp_send(c, (const char*)data, (int)len, vicp_send_cb, p, NULL);
		free(data);
	}
}

static void vicp_test(int argc, char **argv)
{
	int count = 100, ms = 1000;
	int dlen = 1, ret;
	char *dev_name;
	bcp_channel_t *c = NULL;

	if (argc < 4) {
		printf("usage: %s </dev/ttySAC1> <count> <sleep-ms> <data-len>", argv[0]);
		dev_name = (char*)CHANNEL_SERIAL;
	} else {
		dev_name = argv[1];
		count = atoi(argv[2]);
		ms = atoi(argv[3]);
		dlen = atoi(argv[4]);
	}

	c = bcp_channel_create(BCP_CHANNEL_SERIAL, dev_name);
	if (!c) {
		LOG_E("create channel for %s failed.\n", dev_name);
		return;
	}

	if ((ret = c->open(c)) < 0) {
		LOG_E("open channel %s failed, ret = %d.\n", dev_name, ret);
		goto __end;
	}

	if (bcp_vicp_regist_channel(c) < 0) {
		LOG_E("regist channel for %s failed.\n", dev_name);
		goto __end;
	}

	bcp_vicp_regist_data_arrived_callback(c, vicp_data_arrived, NULL);

	my_sleep(4 * 1000);
	LOG_I("start sending....\n");

	while (count-- > 0) {
		send_one_packet(c, dev_name, dlen);
		if (ms > 0) {
			my_sleep(ms);
		}
	}

	while (1) {
		my_sleep(1000);
	}

__end:
	if (c) {
		c->close(c);
		bcp_vicp_unregist_channel(c);
		bcp_channel_destroy(c);
	}
}

int main(int argc, char **argv)
{
	int issub;

	bcp_init();

	//execl();
	//serial_test(argc, argv);
	//nmea_test();
	vicp_test(argc, argv);

	if (argc < 2) {
		printf("usage %s {0|1}", argv[0]);
		return -1;
	}

	issub = atoi(argv[1]);
	if (issub) {
		publishs();
	} else {
		subscribes();
	}

	while (1) {
		my_sleep(100);
	}

	bcp_uninit();

	return 0;
}
