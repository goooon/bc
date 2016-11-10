#include "./GpsUploadTask.h"
GpsUploadTask_NTF::GpsUploadTask_NTF() : Task(APPID_GPS_UPLOADING_NTF, true)
{
}

Task* GpsUploadTask_NTF::Create()
{
	return bc_new GpsUploadTask_NTF();
}
#if defined (_WIN32) || defined(_WIN64)
#define SERIAL_DEVNAME "COM8"
#else
#define SERIAL_DEVNAME "/dev/ttySAC2"
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
	void *s;
	void *p;
#if BC_TARGET == BC_TARGET_LINUX
	p = bcp_nmea_create(trace,error);
	if (!p) {
		LOG_I("bcp nmea create failed\n");
		return;
	}

	if (!(s = bcp_serial_open(SERIAL_DEVNAME, 9600, 8, P_NONE, 1))) {
		LOG_I("open %s failed.", SERIAL_DEVNAME);
		bcp_nmea_destroy(p);
		return;
	}
	fire.update(Config::getInstance().getGpsInterval());
	GPSData data;
	bool hasData = false;
	for (;;) {
		ThreadEvent::WaitResult wr = waitForEvent(500);
		if (wr == ThreadEvent::TimeOut){
			GPSData tmp;
			if (getGps(p, s, tmp)) {
				data = tmp;
				hasData = true;
			}
		}
		else if (wr == ThreadEvent::EventOk){
			MessageQueue::Args args;
			if (msgQueue.out(args)) {
				if (args.e == AppEvent::AbortTasks){
					return;
				}
			}
		}
		Timestamp now;
		if (fire < now) {
			if (hasData) {
				hasData = false;
				fire.update(Config::getInstance().getGpsInterval());
				if (GPSDataQueue::getInstance().isFull()) {
					GPSData d;
					GPSDataQueue::getInstance().out(d);
					GPSDataQueue::getInstance().in(data);
				}
				GPSData* next;
				while (next = GPSDataQueue::getInstance().getNext()) {
					if (ntfGps(*next)) {
						GPSDataQueue::getInstance().out(data);
					}
					else {
						break;
					}
				}
			}
		}
		else {
			LOG_W("NO valid gps data");
		}
	}
	bcp_serial_close(s);
	bcp_nmea_destroy(p);
#else
	fire.update(Config::getInstance().getGpsInterval());
	GPSData data;
	data.longitude = 104.06f;
	data.latitude = 30.67f;
	for (;;) {
		ThreadEvent::WaitResult wr = waitForEvent(500);
		if (wr == ThreadEvent::TimeOut) {
			Timestamp now;
			if (fire < now) {
				fire.update(Config::getInstance().getGpsInterval());
				if (GPSDataQueue::getInstance().isFull()) {
					GPSData d;
					GPSDataQueue::getInstance().out(d);
					GPSDataQueue::getInstance().in(data);
				}
				GPSData* next;
				while (next = GPSDataQueue::getInstance().getNext()) {
					if (ntfGps(*next)) {
						GPSDataQueue::getInstance().out(data);
					}
					else {
						break;
					}
				}
			}
		}
		else {
			MessageQueue::Args args;
			if (msgQueue.out(args)){

			}
		}
	}
#endif
}

bool GpsUploadTask_NTF::getGps(void* p, void* s,GPSData& data)
{
	int r;
	char buff[2048] = { 0, };
	bcp_nmea_info_t *info;
	while((r = bcp_serial_read(s, buff,sizeof(buff), 1000)) >= 0)
	{
		if (r > 0) {
			if (bcp_nmea_parse(p, buff, r) > 0) {
				/* has new sentence */
				info = bcp_nmea_info(p);
				if (info) {
					//print_nmea_info(info);
					if (info->sig == 0) {
						//LOG_I("invalid gps data");
					}
					else {
						data.longitude = info->longitude;
						data.latitude = info->latitude;
					}
					return true;
				}
			}
		}
	}
	LOG_E("GPS data not ok;");
	return false;
}

bool GpsUploadTask_NTF::ntfGps(GPSData& data)
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 5, seqID);
	msg.appendIdentity();
	msg.appendTimeStamp();
	msg.appendGPSData(data);
	msg.appendFunctionStatus(0);
	if (!pkg.post(Config::getInstance().pub_topic, 2, 5000)) {
		LOG_E("ntfGps failed");
	}
	else{
		LOG_I("ntfGps() ---> TSP");
	}
	return true;
}

