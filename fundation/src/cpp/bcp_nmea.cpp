#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../dep/nmealib/include/nmealib/context.h"
#include "../../../dep/nmealib/include/nmealib/info.h"
#include "../../../dep/nmealib/include/nmealib/nmath.h"
#include "../../../dep/nmealib/include/nmealib/parser.h"

#include "../inc/dep.h"
#include "../inc/util/Thread.h"
#include "../inc/bcp_nmea.h"

typedef struct nmea_parser_s
{
	NmeaParser parser;
	NmeaInfo info;
	mutex_type mutex;
} nmea_parser_t;

static void lock(mutex_type m)
{
	Thread_lock_mutex(m);
}

static void unlock(mutex_type m)
{
	Thread_unlock_mutex(m);
}

static void trace(const char *str, size_t str_size) {
	LOG_I(str);
}
static void error(const char *str, size_t str_size) {
	LOG_E(str);
}

void *bcp_nmea_create(void (*fun_trace)(const char*,size_t),void(*fun_error)(const char*,size_t))
{
	bcp_nmea_t *n;
	nmea_parser_t *p;

	n = (bcp_nmea_t*)malloc(sizeof(*n));
	if (!n) {
		return NULL;
	} else {
		memset(n, 0, sizeof(*n));
	}

	p = (nmea_parser_t*)malloc(sizeof(*p));
	if (!p) {
		free(n);
		return NULL;
	} else {
		memset(p, 0, sizeof(*p));
	}

	p->mutex = Thread_create_mutex();
	if (!p->mutex) {
		LOG_W("bcp_nmea_create create mutex failed");
		free(p);
		free(n);
		return NULL;
	}

	nmeaContextSetTraceFunction(fun_trace ? fun_trace : &trace);
	nmeaContextSetErrorFunction(fun_error ? fun_error : &error);

	nmeaInfoClear(&p->info);
	nmeaParserInit(&p->parser, 0);

	n->parser = p;

	return n;
}

static void nmea_to_info(bcp_nmea_info_t *info, NmeaInfo *oinfo)
{
	int i;

	info->present = oinfo->present;
	info->smask = oinfo->smask;
	info->localtime.year = oinfo->localtime.year;
	info->localtime.mon = oinfo->localtime.mon;
	info->localtime.day = oinfo->localtime.day;
	info->localtime.hour = oinfo->localtime.hour;
	info->localtime.min = oinfo->localtime.min;
	info->localtime.sec = oinfo->localtime.sec;
	info->localtime.milisec = oinfo->localtime.hsec;

	info->utc.year = oinfo->utc.year;
	info->utc.mon = oinfo->utc.mon;
	info->utc.day = oinfo->utc.day;
	info->utc.hour = oinfo->utc.hour;
	info->utc.min = oinfo->utc.min;
	info->utc.sec = oinfo->utc.sec;
	info->utc.milisec = oinfo->utc.hsec;

	info->pdop = oinfo->pdop;
	info->hdop = oinfo->hdop;
	info->vdop = oinfo->vdop;
	
	info->latitude = oinfo->latitude;
	info->longitude = oinfo->longitude;
	info->elevation = oinfo->elevation;

	info->height = oinfo->height;
	info->speed = oinfo->speed;

	info->track = oinfo->track;
	info->mtrack = oinfo->mtrack;
	info->magvar = oinfo->magvar;
	
	info->dgpsAge = oinfo->dgpsAge;
	info->dgpsSid = oinfo->dgpsSid;

	info->satellites.use_count = oinfo->satellites.inUseCount;
	info->satellites.view_count = oinfo->satellites.inViewCount;
	for (i = 0; i < NMEA_MAX_SATELLITES && i < NMEALIB_MAX_SATELLITES; i++) {
		info->satellites.use[i] = oinfo->satellites.inUse[i];
		info->satellites.view[i].prn = oinfo->satellites.inView[i].prn;
		info->satellites.view[i].elevation = oinfo->satellites.inView[i].elevation;
		info->satellites.view[i].azimuth = oinfo->satellites.inView[i].azimuth;
		info->satellites.view[i].snr = oinfo->satellites.inView[i].snr;
	}

	info->progress.gpgsv_inprogress = oinfo->progress.gpgsvInProgress;
	info->metric = oinfo->metric;
}

int bcp_nmea_parse(void *hdl, const char *buf, int len)
{
	int ret;
	bcp_nmea_t *n;
	nmea_parser_t *p;

	if (!hdl || !buf) {
		return -1;
	}

	n = (bcp_nmea_t*)hdl;
	p = (nmea_parser_t*)n->parser;

	lock(p->mutex);
	ret = (int)nmeaParserParse(&p->parser, buf, (size_t)len, &p->info);
	unlock(p->mutex);

	return ret;
}

bcp_nmea_info_t *bcp_nmea_info(void *hdl)
{
	bcp_nmea_t *n;
	nmea_parser_t *p;

	if (!hdl) {
		return NULL;
	} else {
		n = (bcp_nmea_t*)hdl;
		p = (nmea_parser_t*)n->parser;
		if (!p) {
			return NULL;
		}
	}

	lock(p->mutex);
	nmea_to_info(&n->info, &p->info);
	unlock(p->mutex);

	return &n->info;
}

const char *bcp_nmea_sig_to_string(int sig)
{
	return nmeaInfoSignalToString((NmeaSignal)sig);
}

const char *bcp_nmea_fix_to_string(int fix)
{
	return nmeaInfoFixToString((NmeaFix)fix);
}

void bcp_nmea_destroy(void *hdl)
{
	bcp_nmea_t *n;
	nmea_parser_t *p;

	if (hdl) {
		n = (bcp_nmea_t*)hdl;
		if (n->parser) {
			p = (nmea_parser_t*)n->parser;
			nmeaParserDestroy(&p->parser);
			Thread_destroy_mutex(p->mutex);
			free(n->parser);
			n->parser = NULL;
		}
		free(hdl);
	}
}

