/* Copyright (c) 2011-2014, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation, nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#define LOG_TAG "beeCloud GPS"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <hardware/gps.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <cutils/properties.h>
#include <math.h>

#include <nmealib/info.h>

#include "VICPClient.h"
#include "location.h"

#include "../../inc/dep.h"
#include "../../inc/Application.h"

namespace android {

#define MAX_NMEA_LEN 256

static GpsCallbacks *acbs = NULL;
static void *parser = NULL;
static int to_stop = 1;
static mutex_type mutex = NULL;
static int has_nmea = 0;
static sem_type sem = NULL;
static bcp_nmea_info_t nmeaInfo;
static char nmeaString[MAX_NMEA_LEN] = {0,};

static void trace(const char *str, size_t str_size) {
	LOG_V(str);
}
static void error(const char *str, size_t str_size) {
	LOG_E(str);
}

static void gps_sv_notify(bcp_nmea_info_t *info)
{
	int i;
	GpsSvStatus status;

	if (!(info->present & NMEALIB_PRESENT_SATINVIEW) ||
		info->progress.gpgsv_inprogress) {
		return;
	}

	status.size = sizeof(GpsSvStatus);
	status.num_svs = (int)info->satellites.view_count;
	if (status.num_svs > GPS_MAX_SVS) {
		status.num_svs = GPS_MAX_SVS;
	}
	for (i = 0; i < status.num_svs; i++) {
		status.sv_list[i].size = sizeof(GpsSvInfo);
		status.sv_list[i].prn = info->satellites.view[i].prn;
		status.sv_list[i].snr = (float)info->satellites.view[i].snr;
		status.sv_list[i].elevation = (float)info->satellites.view[i].elevation;
		status.sv_list[i].azimuth= (float)info->satellites.view[i].azimuth;
		/* ??? */
		status.almanac_mask |= (1ul << (status.sv_list[i].prn - 1));
		status.ephemeris_mask |= (1ul << (status.sv_list[i].prn - 1));
		status.used_in_fix_mask |= (1ul << (status.sv_list[i].prn - 1));
	}

	if (acbs && acbs->sv_status_cb) {
		ALOGI("%s, num_svs: %d", __FUNCTION__, status.num_svs);
		acbs->sv_status_cb(&status);
	}
}

static GpsUtcTime get_timestamp(bcp_nmea_info_t *info)
{
    struct tm tm;
    time_t fix_time, now;
	nmea_time_t *t = &info->utc;

	if (!(info->present & NMEALIB_PRESENT_UTCDATE)) {
        now = time(NULL);
        gmtime_r(&now, &tm);
        t->year = tm.tm_year + 1900;
        t->mon  = tm.tm_mon + 1;
        t->day  = tm.tm_mday;
	}
	if (!(info->present & NMEALIB_PRESENT_UTCTIME)) {
        now = time(NULL);
        gmtime_r(&now, &tm);
        t->hour = tm.tm_hour;
        t->min  = tm.tm_min;
        t->sec  = tm.tm_sec;
		t->milisec = 0;
	}

    tm.tm_hour  = (int)t->hour;
    tm.tm_min   = (int)t->min;
    tm.tm_sec   = (int)t->sec;
    tm.tm_year  = (int)t->year - 1900;
    tm.tm_mon   = (int)t->mon - 1;
    tm.tm_mday  = (int)t->day;
    tm.tm_isdst = -1;

    fix_time = mktime( &tm ) + 0/*utc_diff*/;
    return (int64_t)fix_time * 1000LL;
}

static double
convert_from_hhmm(double val)
{
    int degrees = (int)(floor(val) / 100);
    double minutes = val - degrees*100.;
    double dcoord  = degrees + minutes / 60.0;
    return dcoord;
}

static void gps_location_notify(bcp_nmea_info_t *info)
{
	GpsLocation loc;

	if (!(info->present & NMEALIB_PRESENT_SIG)) {
		return;
	}

	loc.size = sizeof(GpsLocation);
	loc.flags = 0;
	if ((info->present & NMEALIB_PRESENT_LAT) | 
		(info->present & NMEALIB_PRESENT_LON)) {
		loc.flags |= GPS_LOCATION_HAS_LAT_LONG;
	}
	if (info->present & NMEALIB_PRESENT_HEIGHT) {
		loc.flags |= GPS_LOCATION_HAS_ALTITUDE;
	}
	if (info->present & NMEALIB_PRESENT_SPEED) {
		loc.flags |= GPS_LOCATION_HAS_SPEED;
	}
	if (info->present & NMEALIB_PRESENT_TRACK) {
		loc.flags |= GPS_LOCATION_HAS_BEARING;
	}
	if (info->present & NMEALIB_PRESENT_PDOP) {
		loc.flags |= GPS_LOCATION_HAS_ACCURACY;
	}
	if (info->latitude < 0.0f) {
		loc.latitude = convert_from_hhmm(-info->latitude);
		loc.latitude = -loc.latitude;
	} else {
		loc.latitude = convert_from_hhmm(info->latitude);
	}
	if (info->longitude < 0.0f) {
		loc.longitude = convert_from_hhmm(-info->longitude);
		loc.longitude = -loc.longitude;
	} else {
		loc.longitude = convert_from_hhmm(info->longitude);
	}
	loc.altitude = info->elevation;
	loc.speed = (float)info->speed;
	loc.bearing = (float)info->track;
	loc.accuracy = (float)info->pdop;
	loc.timestamp = get_timestamp(info);
	if (acbs && acbs->location_cb) {
		ALOGI("%s, %f, %f, %f, %f, %f, %f, 0x%llx", __FUNCTION__, 
			loc.latitude, loc.longitude, loc.altitude, 
			loc.speed, loc.bearing, loc.accuracy, loc.timestamp);
		acbs->location_cb(&loc);
	}
}

static void gps_nmea_notify(bcp_nmea_info_t *info, const char *s)
{
	if (!(info->present & NMEALIB_PRESENT_SIG)) {
		return;
	}
	if (acbs && acbs->nmea_cb) {
		ALOGI("nmea: %s", s);
		acbs->nmea_cb(get_timestamp(info), s, strlen(s));
	}
}

static void gps_notify(bcp_nmea_info_t *info, const char *s)
{
	gps_sv_notify(info);
	gps_location_notify(info);
	gps_nmea_notify(info, s);
}

static void notify(void)
{
	bcp_nmea_info_t info;
	char s[MAX_NMEA_LEN] = {0,};

	Thread_lock_mutex(mutex);
	if (has_nmea) {
		has_nmea = 0;
		info = nmeaInfo;
		strcpy(s, nmeaString);
		Thread_unlock_mutex(mutex);
		gps_notify(&info, s);
		Thread_lock_mutex(mutex);
	}
	Thread_unlock_mutex(mutex);
}

static void gps_notify_thread(void *arg)
{
	Thread_lock_mutex(mutex);
	to_stop = 0;
	while (!to_stop) {
		Thread_unlock_mutex(mutex);
		Thread_wait_sem(sem, 2000);
		notify();
		Thread_lock_mutex(mutex);
	}
	to_stop = 2;
	Thread_unlock_mutex(mutex);
}

class GPSListener:
	public VICPListener
{
	void data_arrived(int32_t app_id, uint8_t *buf, int32_t len)
	{
		bcp_nmea_info_t *info;
		LOG_I("%s gps, len: %d", __FUNCTION__, len);
		if (bcp_nmea_parse(parser, (const char*)buf, len) > 0) {
			info = bcp_nmea_info(parser);
			if (info) {
				Thread_lock_mutex(mutex);
				nmeaInfo = *info;
				if (len >= MAX_NMEA_LEN) {
					len = MAX_NMEA_LEN - 1;
				}
				strncpy(nmeaString, (const char*)buf, len);
				nmeaString[len - 1] = 0;
				has_nmea = 1;
				Thread_unlock_mutex(mutex);
				Thread_post_sem(sem);
			}
		}
		free(buf);
	}
};

static void joinThreadPool(void)
{
	sp<ProcessState> ps(ProcessState::self());
	ps->startThreadPool();
	ps->giveThreadPoolName();
	IPCThreadState::self()->joinThreadPool();
}

static const int32_t APPID_GPS = 0x8000;
static int startListener(int32_t app_id, 
	sp<GPSListener>& listener)
{
	sp<IServiceManager> sm = defaultServiceManager();
	sp<IBinder> binder =
		sm->getService(String16("VICPSystem"));
	sp<IVICPSystem> service =
		interface_cast<IVICPSystem>(binder);

	if (service == NULL) {
		ALOGE("get VICPSystem service failed");
		return -1;
	}
	service->registerListener(app_id, listener);
	//joinThreadPool();

	to_stop = 0;
	while (!to_stop) {
		msleep(1000);
	}

	return 0;
}

class clientLoop : public ::Thread
{
	virtual void run() OVERRIDE
	{
		sp<GPSListener> listener = new GPSListener();
		startListener(APPID_GPS, listener);
	}
};

static void status_notify(GpsStatusValue s)
{
	GpsStatus status;
	if (acbs && acbs->status_cb) {
		status.size = sizeof(GpsStatus);
		status.status = s;
		acbs->status_cb(&status);
	}
}

static int loc_init(GpsCallbacks* callbacks)
{
	ALOGI("%s", __FUNCTION__);
	acbs = callbacks;
	has_nmea = 0;
	if (acbs) {
		parser = bcp_nmea_create(trace, error);
		if (!parser)
			return -1;
		mutex = Thread_create_mutex();
		if (!mutex) {
			bcp_nmea_destroy(parser);
			return -1;
		}
		sem = Thread_create_sem();
		if (!sem) {
			bcp_nmea_destroy(parser);
			Thread_destroy_mutex(mutex);
			return -1;
		}
		::Thread::startThread(new clientLoop());
		acbs->create_thread_cb("gps_notify_thread", 
			gps_notify_thread, NULL);
		status_notify(GPS_STATUS_ENGINE_ON);
	}
	return 0;
}

static void loc_cleanup(void)
{
	ALOGI("%s", __FUNCTION__);
	status_notify(GPS_STATUS_ENGINE_OFF);
	if (!mutex) {
		return;
	}
	Thread_lock_mutex(mutex);
	to_stop = 1;
	while (to_stop != 2) {
		Thread_unlock_mutex(mutex);
		msleep(100);
		Thread_lock_mutex(mutex);
	}
	Thread_unlock_mutex(mutex);
	if (parser)
		bcp_nmea_destroy(parser);
	Thread_destroy_mutex(mutex);
	Thread_destroy_sem(sem);
	mutex = NULL;
	sem = NULL;
	parser = NULL;
	acbs = NULL;
}

static int loc_start(void)
{
	ALOGI("%s", __FUNCTION__);
	status_notify(GPS_STATUS_SESSION_BEGIN);
	return 0;
}

static int loc_stop(void)
{
	ALOGI("%s", __FUNCTION__);
	status_notify(GPS_STATUS_SESSION_END);
	return 0;
}

static int  loc_set_position_mode(GpsPositionMode mode,
                                  GpsPositionRecurrence recurrence,
                                  uint32_t min_interval,
                                  uint32_t preferred_accuracy,
                                  uint32_t preferred_time)
{
	ALOGI("%s, mode: %d", __FUNCTION__, mode);
	return 0;
}

static int loc_inject_time(GpsUtcTime time, int64_t timeReference, int uncertainty)
{
	ALOGI("%s", __FUNCTION__);
	return 0;
}

static int loc_inject_location(double latitude, double longitude, float accuracy)
{
	ALOGI("%s", __FUNCTION__);
	return 0;
}

static void loc_delete_aiding_data(GpsAidingData f)
{
	ALOGI("%s", __FUNCTION__);
}

const void* loc_get_extension(const char* name)
{
	ALOGI("%s", __FUNCTION__);
	return NULL;
}

static const GpsInterface sLocEngInterface =
{
   sizeof(GpsInterface),
   loc_init,
   loc_start,
   loc_stop,
   loc_cleanup,
   loc_inject_time,
   loc_inject_location,
   loc_delete_aiding_data,
   loc_set_position_mode,
   loc_get_extension
};
}

extern "C" const GpsInterface* get_gps_interface()
{
    return &android::sLocEngInterface;
}

