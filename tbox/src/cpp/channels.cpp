#include "../inc/Application.h"
#include "../inc/channels.h"
#include "../tasks/TaskTable.h"
#include "../../../fundation/src/inc/vicp/bcp_vicp.h"

#define __countof(a) (sizeof(a) / sizeof(a[0]))

typedef struct channel_conf_s {
	int type;
	const char *name;
	const char *dev_name;
	bcp_channel_t *ch;
} channel_conf_t;

static channel_conf_t confs[] = {
	{BCP_CHANNEL_SERIAL, "serial", "/dev/ttyCh", NULL} // tbox <--> mp5
};

static void packet_arrived(void *context, u8 *buf, u16 len)
{
	u64 seq_id;
	u32 step_id;
	u16 app_id = 0;
	bool done;
	Task* task = NULL;
	bcp_packet_t *p;

	bcp_message_t *m = NULL;
	bcp_element_t *e = NULL;

	if (bcp_packet_unserialize((u8*)buf, (u32)len, &p) < 0) {
		LOG_E("bcp_packet_unserialize failed");
		free(buf);
		return;
	}

	while ((m = bcp_next_message(p, m)) != NULL) {
		app_id = m->hdr.id;
		seq_id = m->hdr.sequence_id;
		step_id = m->hdr.step_id;
		LOG_I("vicp received app_id:%d, step_id:%d", 
			app_id, step_id);
		task = Application::getInstance().findTask(app_id);
		if (task != NULL) {
			done = task->handlePackage(p);
			if (!done) {
				::PostEvent(AppEvent::AbortTasks, app_id, 0, p);
			}
		} else {
			::PostEvent(AppEvent::InsertTask, 0, 0, TaskCreate(app_id, p));
		}
	}
}

static int regist_channel(channel_conf_t *c)
{
	bcp_channel_t *ch;

	ch = bcp_channel_create(c->type, c->name, c->dev_name);
	if (!ch) {
		LOG_E("bcp channel create failed, %s: %s", 
			c->name, c->dev_name);
		return -1;
	}
	if (ch->open(ch) < 0) {
		bcp_channel_destroy(ch);
		LOG_E("bcp channel open failed, %s: %s", 
			c->name, c->dev_name);
		return -1;
	}
	if (bcp_vicp_regist_channel(ch) < 0) {
		ch->close(ch);
		bcp_channel_destroy(ch);
		LOG_E("bcp channel regist failed, %s: %s", 
			c->name, c->dev_name);
		return -1;
	}

	bcp_vicp_regist_data_arrived_callback(ch, packet_arrived, c);
	c->ch = ch;

	return 0;
}

static int unregist_channel(channel_conf_t *c)
{
	bcp_channel_t *ch = c->ch;

	if (!ch) {
		return -1;
	}

	if (bcp_vicp_unregist_channel(ch) < 0) {
		ch->close(ch);
		bcp_channel_destroy(ch);
		LOG_E("bcp channel unregist failed, %s: %s", 
			c->name, c->dev_name);
		return -1;
	}

	if (ch->close(ch) < 0) {
		bcp_channel_destroy(ch);
		LOG_E("bcp channel close failed, %s: %s", 
			c->name, c->dev_name);
		return -1;
	}

	bcp_channel_destroy(ch);
	c->ch = NULL;

	return 0;
}

void channels_init(void)
{
	int i;

	for (i = 0; i < __countof(confs); i++) {
		regist_channel(&confs[i]);
	}
}

bcp_channel_t *channels_get(const char *name)
{
	return bcp_channel_get_byname(name);
}

void channels_put(bcp_channel_t *c)
{
	return bcp_channel_put(c);
}

void channels_uinit(void)
{
	int i;

	for (i = 0; i < __countof(confs); i++) {
		unregist_channel(&confs[i]);
	}
}

