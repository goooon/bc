#include "./GpsUploadTask.h"
GpsUploadTask::GpsUploadTask() : Task(APPID_GPS_UPLOADING_NTF, true)
{
}

Task* GpsUploadTask::Create()
{
	return bc_new GpsUploadTask();
}
#if defined (_WIN32) || defined(_WIN64)
#define SERIAL_DEVNAME "COM6"
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

void GpsUploadTask::doTask()
{
	void *s;
	void *p;
	
	p = bcp_nmea_create();
	if (!p) {
		LOG_I("bcp nmea create failed\n");
		return;
	}

	if (!(s = bcp_serial_open(SERIAL_DEVNAME, 9600, 8, P_NONE, 1))) {
		LOG_I("open %s failed.\n", SERIAL_DEVNAME);
		bcp_nmea_destroy(p);
		return;
	}
	for (;;) {
		ThreadEvent::WaitResult wr = waitForEvent(Config::getInstance().getGpsInterval());
		if (wr == ThreadEvent::TimeOut)
		{
			GPSData data;
			if (getGps(p, s, data)) {
				if (!ntfGps(data)) {
					GPSDataQueue::getInstance().in(data);
				}
			}
		}
	}
	bcp_serial_close(s);
	bcp_nmea_destroy(p);
}

bool GpsUploadTask::getGps(void* p, void* s,GPSData& data)
{
	int r;
	char buff[2048] = { 0, };
	bcp_nmea_info_t *info;
	if((r = bcp_serial_read(s, buff, 1, 1000)) >= 0)
	{
		if (r > 0) {
			if (bcp_nmea_parse(p, buff, 1) > 0) {
				/* has new sentence */
				info = bcp_nmea_info(p);
				if (info) {
					print_nmea_info(info);
					data.langitude = 0;
					data.longitude = 1;
					return true;
				}
			}
		}
	}
	LOG_E("GPS data not ok;");
	return false;
}

bool GpsUploadTask::ntfGps(GPSData& data)
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

