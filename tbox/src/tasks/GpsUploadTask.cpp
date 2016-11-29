#include <math.h>
#include "./GpsUploadTask.h"
#include "../inc/Vehicle.h"
#include "../inc/channels.h"
#include "../../../fundation/src/inc/binary_formater.h"

#undef TAG
#define TAG "GPS"

#if !defined(_WIN32) && !defined(_WIN64)
#include <unistd.h>
#define WINAPI
#else
#include <windows.h>
#endif
#if defined (_WIN32) || defined(_WIN64)
#define SERIAL_DEVNAME "COM8"
#define CHANNEL_NAME "serial"
#else
#define SERIAL_DEVNAME "/dev/ttySAC2"
#define CHANNEL_NAME "serial"
#endif

#if BC_TARGET == BC_TARGET_LINUX

static mutex_type mutex = NULL;
static int stop = 1;
static bf_t bf;

static void gps_send_cb(void *context, int result)
{
	if (result != 0) {
		LOG_W("gps_send_cb result = %d\n", result);
	}
}

static void gps_send(bcp_channel_t *ch, const char *buf, int len)
{
	bcp_packet_t *p;
	u8 *out;
	u32 olen;

	p = bcp_create_one_message(APPID_VIS_GPS, 0, 
		bcp_next_seq_id(), (u8*)buf, len);
	if (p) {
		if (bcp_packet_serialize(p, &out, &olen) >= 0) {
			bcp_vicp_send(ch, (const char*)out, (int)olen, 
				gps_send_cb, NULL, NULL);
			free(out);
		}
		bcp_packet_destroy(p);
	}
}

static void push_buff(const char *buf, int len)
{
	Thread_lock_mutex(mutex);
	bf_put_bytes_only(&bf, (u8*)buf, (u32)len);
	Thread_unlock_mutex(mutex);
}

static char *pop_buff(void)
{
	char *p = NULL;
	Thread_lock_mutex(mutex);
	bf_read_bytes_only(&bf, (u8**)&p, bf.index);
	bf_reset(&bf, 0);
	Thread_unlock_mutex(mutex);
	return p;
}

static thread_return_type WINAPI gps_thread(void *arg)
{
	int r;
	void *s;
	bcp_channel_t *ch;
	char buff[100] = {0, };

	s = bcp_serial_open(SERIAL_DEVNAME, 9600, 8, P_NONE, 1);
	if (!s) {
		return NULL;
	}
	ch = channels_get(CHANNEL_NAME);
	if (!ch) {
		bcp_serial_close(s);
		return NULL;
	}

	Thread_lock_mutex(mutex);
	stop = 0;

	while (!stop) {
		Thread_unlock_mutex(mutex);
		r = bcp_serial_read(s, buff, sizeof(buff), 1000);
		if (r > 0) {
			push_buff(buff, r);
			gps_send(ch, buff, r);
		}
		Thread_lock_mutex(mutex);
	}

	stop = 2;
	Thread_unlock_mutex(mutex);

	channels_put(ch);
	bcp_serial_close(s);

	return NULL;
}

static int start_thread()
{
	Thread_start(gps_thread, NULL);

	Thread_lock_mutex(mutex);
	while (stop != 0) {
		Thread_unlock_mutex(mutex);
		msleep(100);
		Thread_lock_mutex(mutex);
	}
	Thread_unlock_mutex(mutex);

	return 0;
}
static int stop_thread()
{
	Thread_lock_mutex(mutex);
	stop = 1;
	while (stop != 2) {
		Thread_unlock_mutex(mutex);
		msleep(100);
		Thread_lock_mutex(mutex);
	}
	Thread_unlock_mutex(mutex);

	return 0;
}

static void gps_thread_start(void)
{
	bf_init(&bf, NULL, 0);
	mutex = Thread_create_mutex();
	stop = 1;
	start_thread();
}
static void gps_thread_stop(void)
{
	stop_thread();
	bf_uninit(&bf, 1);
	Thread_destroy_mutex(mutex);
}
#endif

GpsUploadTask_NTF::GpsUploadTask_NTF(u32 appId) : Task(appId, true)
{
	longPrev = 0;
	latiPrev = 0;
}

Task* GpsUploadTask_NTF::Create(u32 appId)
{
	return bc_new GpsUploadTask_NTF(appId);
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

static void trace(const char *str, size_t str_size) {
	LOG_V(str);
}
static void error(const char *str, size_t str_size) {
	LOG_E(str);
}

void GpsUploadTask_NTF::sendGps(GPSDataQueue::GPSInfo& info, Vehicle::RawGps & rawGps)
{
	Timestamp now;
	if (!Vehicle::getInstance().isIgnited() &&
		!Vehicle::getInstance().isMovingInAbnormal()) {
		if (calcDistance(longPrev, latiPrev, rawGps) > Config::getInstance().getAbnormalMovingStartDistanceLimit()) {
			Vehicle::getInstance().setMovingInAbnormal(true);
			LOG_W("Vehicle is moving in abnormal");
			ntfEnterAbnormal();
			abnormalPrev.update();
			longPrev = rawGps.longitude;
			latiPrev = rawGps.latitude;
			info.appId = APPID_GPS_ABNORMAL_MOVE;
			sendGpsData(info);
		}
	}
	if (Vehicle::getInstance().isMovingInAbnormal()) {
		if (needSendAbnormalGps(rawGps)) {
			info.appId = APPID_GPS_ABNORMAL_MOVE;
			longPrev = rawGps.longitude;
			latiPrev = rawGps.latitude;
			sendGpsData(info);
		}
	}
	else if (normalToFire < now) {
		normalToFire.update(Config::getInstance().getGpsInterval());
		info.appId = Vehicle::getInstance().isIgnited() ? APPID_GPS_UPLOADING_NTF_MOVE : APPID_GPS_UPLOADING_NTF_CONST;
		sendGpsData(info);
	}
}

void GpsUploadTask_NTF::doTask()
{
	void *p = 0;

#if BC_TARGET == BC_TARGET_LINUX
	gps_thread_start();
#endif

	p = bcp_nmea_create(trace, error);
	if (!p) {
		LOG_I("bcp nmea create failed\n");
		return;
	}

	GPSDataQueue::GPSInfo info;
	Vehicle::RawGps rawGps;
	Vehicle::getInstance().getGpsInfo(rawGps);
	if (getGps(p, info, rawGps)) {
		Vehicle::getInstance().setGpsInfo(rawGps);
		longPrev = rawGps.longitude;
		latiPrev = rawGps.latitude;
	}
	else {
		longPrev = rawGps.longitude;
		latiPrev = rawGps.latitude;
	}
	normalToFire.update(Config::getInstance().getGpsInterval());
	for (;;) {
		ThreadEvent::WaitResult wr = waitForEvent(500);
		if (wr == ThreadEvent::TimeOut) {
			if (getGps(p, info, rawGps)) {
				Vehicle::getInstance().setGpsInfo(rawGps);
			}
			else {
				LOG_W("No GPS Data Valid");
				continue;
			}
			sendGps(info,rawGps);
		}
		else {
			MessageQueue::Args args;
			if (msgQueue.out(args)){
				sendGps(info, rawGps);
			}
		}
	}
	bcp_nmea_destroy(p);
#if BC_TARGET == BC_TARGET_LINUX
	gps_thread_stop();
#endif
}

void GpsUploadTask_NTF::sendGpsData(GPSDataQueue::GPSInfo info)
{
	if (GPSDataQueue::getInstance().isFull()) {
		GPSDataQueue::GPSInfo d;
		GPSDataQueue::getInstance().out(d);
		GPSDataQueue::getInstance().in(info);
	}
	else {
		GPSDataQueue::getInstance().in(info);
	}
	GPSDataQueue::GPSInfo* next;
	while (next = GPSDataQueue::getInstance().getNext()) {
		if (Vehicle::getInstance().isAuthed()) {
			if (ntfGps(*next)) {
				GPSDataQueue::getInstance().out(info);
			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}
}

static void RawGps2AutoLocation(Vehicle::RawGps& rawGps, AutoLocation& loc) {
	u32 Longitude =  (180.f + rawGps.longitude) * 1000000;
	u32 Latitude =  (90.0f + rawGps.latitude) * 1000000;
	u32 Altitude = (rawGps.altitude * 10) + 100000;
	u32 speed = (rawGps.speed) * 100;
	u32 angle = rawGps.dirAngle * 1000000;
	loc.set(Latitude, Longitude, Altitude, speed, 0);
	loc.SatelliteNumber = rawGps.satelliteNumber;
}

bool GpsUploadTask_NTF::getGps(void* p, GPSDataQueue::GPSInfo& gpsinfo, Vehicle::RawGps& rawGps)
{
	gpsinfo.ts.update();
#if BC_TARGET == BC_TARGET_LINUX
	int r;
	char *buf = NULL;
	bcp_nmea_info_t *info;

	buf = pop_buff();
	if (buf) {
		if (bcp_nmea_parse(p, buf, r) > 0) {
			/* has new sentence */
			info = bcp_nmea_info(p);
			if (info) {
				//print_nmea_info(info);
				if (info->sig == 0) {
					//LOG_I("invalid gps data");
					Vehicle::getInstance().setIsGpsValid(false);
				}
				else {
					Vehicle::getInstance().setIsGpsValid(true);
					rawGps.longitude = info->longitude;
					rawGps.latitude = info->latitude;
					rawGps.satelliteNumber = (info->satellites.use_count);
					rawGps.altitude = info->elevation;
					rawGps.dirAngle = info->mtrack;
					rawGps.speed = info->speed;
					RawGps2AutoLocation(rawGps, gpsinfo.location);
				}
				return true;
			}
		}
		free(buf);
	}
	LOG_E("GPS data not ok;");
	return false;
#else
	Vehicle::getInstance().getGpsInfo(rawGps);
	RawGps2AutoLocation(rawGps, gpsinfo.location);
	return true;
#endif
}

bool GpsUploadTask_NTF::ntfGps(GPSDataQueue::GPSInfo& info)
{
	BCPackage pkg;
	u32 stepId = info.appId == APPID_GPS_ABNORMAL_MOVE ? 3 : NTF_STEP_ID;
	BCMessage msg = pkg.appendMessage(info.appId, stepId, Vehicle::getInstance().getTBoxSequenceId());
	msg.appendIdentity();
	msg.appendTimeStamp(&info.ts);
	msg.appendGPSData(info.location);
	if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut())) {
		LOG_E("ntfGps(%d) failed",info.appId);
		return false;
	}
	else{
		Vehicle::RawGps rawGps;
		Vehicle::getInstance().getGpsInfo(rawGps);
		LOG_I("ntfGps(%d£¬%d) lon:%.7f,lat:%.7f ---> TSP",info.appId, stepId, rawGps.longitude, rawGps.latitude);
	}
	return true;
}

bool GpsUploadTask_NTF::ntfEnterAbnormal()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(APPID_GPS_ABNORMAL_MOVE, 1, seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
	msg.appendAutoAlarm(1);//start
	if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut())) {
		LOG_E("ntfEnterAbnormal() failed");
		return false;
	}
	else {
		LOG_I("ntfEnterAbnormal() ---> TSP");
		return true;
	}
}

bool GpsUploadTask_NTF::ntfExitAbnormal()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(APPID_GPS_ABNORMAL_MOVE, NTF_STEP_ID, seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
	msg.appendAutoAlarm(2);//stop
	if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut())) {
		LOG_E("ntfExitAbnormal() failed");
		return false;
	}
	else {
		LOG_I("ntfExitAbnormal() ---> TSP");
		return true;
	}
}

bool GpsUploadTask_NTF::needSendAbnormalGps(Vehicle::RawGps& rawGps)
{
	double dist = calcDistance(longPrev, latiPrev, rawGps);
	dist = dist < 0.0f ? -dist : dist;
	Timestamp now;
	if (dist >= Config::getInstance().getAbnormalMovingStartDistanceLimit()) {
		if (now - abnormalPrev >= Config::getInstance().getAbnormalMovingDuration()){
			abnormalPrev = now;
			return true;
		}
		else {
			//wait for remaining time
		}
	}
	else if (now - abnormalPrev >= Config::getInstance().getAbnormalMovingStopTimeLimit()){
		if (dist < Config::getInstance().getAbnormalMovingStopDistanceLimit()) {
			LOG_I("Vehicle exit abnormal movting status");
			Vehicle::getInstance().setMovingInAbnormal(false);
			ntfExitAbnormal();
			return false;
		}
		else{
			abnormalPrev = now;
			return true;
		}
		
	}
	return false;
}
#define PI 3.1415926535898

double GpsUploadTask_NTF::calcDistance(double long1, double lat1, Vehicle::RawGps& rawGps)
{
	double long2 = rawGps.longitude;
	double lat2 = rawGps.latitude;
	double a, b, R;
	R = 6378137; //µØÇò°ë¾¶
	lat1 = lat1 * PI / 180.0f;
	lat2 = lat2 * PI / 180.0f;
	a = lat1 - lat2;
	b = (long1 - long2) * PI / 180.0f;
	double d;
	double sa2, sb2;
	sa2 = sin(a / 2.0);
	sb2 = sin(b / 2.0);
	d = 2 * R * asin(sqrt(sa2 * sa2 + cos(lat1) * cos(lat2) * sb2 * sb2));
	return d;
}
//APPID_GPS_UPLOADING
GpsUploadTask::GpsUploadTask(u32 appId) : Task(appId, true)
{
}

Task* GpsUploadTask::Create(u32 appId)
{
	return bc_new GpsUploadTask(appId);
}

void GpsUploadTask::doTask()
{
	RspAck();
	Vehicle::RawGps data;
	Vehicle::getInstance().getGpsInfo(data);
	ntfGps(data);
}

bool GpsUploadTask::ntfGps(Vehicle::RawGps& rawGps)
{
	AutoLocation data;
	BCPackage pkg;

	RawGps2AutoLocation(rawGps, data);
	BCMessage msg = pkg.appendMessage(appID, NTF_STEP_ID , seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
	msg.appendErrorElement(0);
	msg.appendGPSData(data);
	LOG_I("ntfGps(%d) ---> TSP",appID);
	if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut())) {
		LOG_E("ntfGps failed");
		return false;
	}
	return true;
}
