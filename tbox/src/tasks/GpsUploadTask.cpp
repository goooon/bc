#include <math.h>
#include "./GpsUploadTask.h"
#include "../inc/Vehicle.h"
#include "../inc/channels.h"

#undef TAG
#define TAG "GPS"

//#undef BC_TARGET
//#define BC_TARGET BC_TARGET_LINUX

GpsUploadTask_NTF::GpsUploadTask_NTF() : Task(APPID_GPS_UPLOADING_NTF, true)
{
	longPrev = 0;
	latiPrev = 0;
}

Task* GpsUploadTask_NTF::Create()
{
	return bc_new GpsUploadTask_NTF();
}
#if defined (_WIN32) || defined(_WIN64)
#define SERIAL_DEVNAME "COM8"
#define VICP_SERIAL_NAME "COM7"
#else
#define SERIAL_DEVNAME "/dev/ttySAC2"
#define VICP_SERIAL_NAME "/dev/ttyCh"
#endif

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

void GpsUploadTask_NTF::doTask()
{
	void *s = 0;
	void *p = 0;
	bcp_channel_t *ch = NULL;
	const char *ch_name = "serial";

#if BC_TARGET == BC_TARGET_LINUX
	ch = channels_get(ch_name);
	if (!ch) {
		LOG_I("find channel failed. name: %s\n", ch_name);
		return;
	}
	p = bcp_nmea_create(trace,error);
	if (!p) {
		channels_put(ch);
		LOG_I("bcp nmea create failed\n");
		return;
	}

	if (!(s = bcp_serial_open(SERIAL_DEVNAME, 9600, 8, P_NONE, 1))) {
		LOG_I("open %s failed.", SERIAL_DEVNAME);
		bcp_nmea_destroy(p);
		channels_put(ch);
		return;
	}
#endif
	GPSDataQueue::GPSInfo info;
	Vehicle::RawGps rawGps;
	Vehicle::getInstance().getGpsInfo(rawGps);
	if (getGps(p, s, ch, info, rawGps)) {
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
			Timestamp now;
			
			if (getGps(p, s, ch, info,rawGps)) {
				Vehicle::getInstance().setGpsInfo(rawGps);
			}
			else {
				LOG_W("No GPS Data Valid");
				continue;
			}
			if (!Vehicle::getInstance().isIgnited() && 
				!Vehicle::getInstance().isMovingInAbnormal()){
				if (calcDistance(longPrev, latiPrev, rawGps) > Config::getInstance().getAbnormalMovingDist()) {
					Vehicle::getInstance().setMovingInAbnormal(true);
					LOG_W("Vehicle is moving in abnormal");
					abnormalPrev.update();
					longPrev = rawGps.longitude;
					latiPrev = rawGps.latitude;
				}
			}
			if (Vehicle::getInstance().isMovingInAbnormal()) {
				info.appId = APPID_GPS_ABNORMAL_MOVE;
				if (needSendAbnormalGps(rawGps)) {
					//LOG_I("send abnormal gps data");
					sendGpsData(info);
				}
			}
			else if (normalToFire < now) {
				normalToFire.update(Config::getInstance().getGpsInterval());
				info.appId = appID;
				//
				sendGpsData(info);
			}
		}
		else {
			MessageQueue::Args args;
			if (msgQueue.out(args)){

			}
		}
	}
#if BC_TARGET == BC_TARGET_LINUX
	bcp_serial_close(s);
	bcp_nmea_destroy(p);
	if (ch) {
		channels_put(ch);
	}
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

#if BC_TARGET == BC_TARGET_LINUX
static void gps_send_cb(void *context, int result)
{
	if (result != 0) {
		LOG_W("gps data sending result = %d\n", result);
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
			bcp_vicp_send(ch, buf, len, gps_send_cb, NULL, NULL);
			free(out);
		}
		bcp_packet_destroy(p);
	}
}
#endif

bool GpsUploadTask_NTF::getGps(void* p, void* s, void *ch, GPSDataQueue::GPSInfo& gpsinfo, Vehicle::RawGps& rawGps)
{
	gpsinfo.ts.update();
#if BC_TARGET == BC_TARGET_LINUX
	int r;
	char buff[2048] = { 0, };
	bcp_nmea_info_t *info;
	bcp_channel_t *bch = (bcp_channel_t*)ch;

	while((r = bcp_serial_read(s, buff,sizeof(buff), 1000)) >= 0)
	{
		if (r > 0) {
			gps_send(bch, buff, r);
			if (bcp_nmea_parse(p, buff, r) > 0) {
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
		}
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
	BCMessage msg = pkg.appendMessage(info.appId, NTF_STEP_ID, seqID);
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
		LOG_I("ntfGps(%d) lon:%.7f,lat:%.7f ---> TSP",info.appId, rawGps.longitude, rawGps.latitude);
	}
	return true;
}

bool GpsUploadTask_NTF::ntfExitAbnormal()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, NTF_STEP_ID, seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
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
	if (dist >= 10.0f) {
		if (now - abnormalPrev >= Config::getInstance().getAbnormalMovingDuration()){
			return true;
		}
	}
	else if (now - abnormalPrev >= Config::getInstance().getDurationEnterNormal()){
		LOG_I("Vehicle exit abnormal movting status");
		Vehicle::getInstance().setMovingInAbnormal(false);
		ntfExitAbnormal();
		return false;
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

GpsUploadTask::GpsUploadTask() : Task(APPID_GPS_UPLOADING, true)
{
}

Task* GpsUploadTask::Create()
{
	return bc_new GpsUploadTask();
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
	RawGps2AutoLocation(rawGps, data);
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, NTF_STEP_ID, seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
	msg.appendErrorElement(0);
	msg.appendGPSData(data);
	
	if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut())) {
		LOG_E("ntfGps failed");
	}
	else {
		LOG_I("ntfGps() ---> TSP");
	}
	return true;
}
