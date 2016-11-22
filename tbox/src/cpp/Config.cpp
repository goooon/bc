#include "../inc/Config.h"
#include "../inc/Application.h"
#include "../inc/Vehicle.h"

#undef TAG
#define TAG "Config"

Config& Config::getInstance()
{
	return Application::getInstance().getConfig();
}

u32 Config::getGpsInterval()
{
	return Vehicle::getInstance().isIgnited() ? gpsIntervalDriving : gpsIntervalStation;
}

bool Config::parse(int argc, char** argv)
{
	memset(pub_topic, 0, sizeof(pub_topic));
	memset(sub_topic, 0, sizeof(sub_topic));
	memset(mqttServerIp, 0, sizeof(mqttServerIp));
	//memset(clientid, 0, sizeof(clientid));
	memset(vin, 0, sizeof(vin));

	mqttQos = 2;
	isGpsDataValid = true;
	gpsTaskAtStartup = true;
	canAuto = true;
	mqttSendTimeOut = 5000;
	gpsQueueSize = 4096;
	abnormalMovingDist = 50;
	unIgnitionNotifyDelay = 1000;
	gpsIntervalAbormal = 10000;
	durationEnterNormal = 30000;
	//abnormalMovingInterval = 10;
	doorDeactiveTimeOut = 5000;
	doorActivationTimeOut = 5000;
	igntActivationTimeOut = 5000;
	mqttReConnInterval = 50000;
	authRetryInterval = 5000;

	gpsIntervalStation = 1000 * 5;
	gpsIntervalDriving = 1000 * 3;
	stateUploadExpireTime = 2 * 1000; //2 min
	strncpy(mqttServerIp, "10.28.4.40:1884", sizeof(mqttServerIp));//main server
	 //strncpy(mqttServer, "139.219.238.66:1883", sizeof(mqttServer));//test server
	 //strncpy(mqttServer, "10.28.248.71:1883", sizeof(mqttServer));//guzhibin

	strncpy(vin, "VIN67423921", sizeof(vin));//main server
	memset(tsn, ' ', sizeof(tsn));
	memset(imei, ' ', sizeof(imei));
	memset(iccid, ' ', sizeof(iccid));

	keepAliveInterval = 20;
	startChannel = false;

	//////////////////////////////////////////////////////////////////////////
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-ip")) {
			if (i + 1 == argc) {
				showCmdLine();
				return false;
			}
			strncpy(mqttServerIp, argv[i + 1], sizeof(mqttServerIp));//main server
			i++;
		}
		else if (!strcmp(argv[i], "-pub")) {
			if (i + 1 == argc) {
				showCmdLine();
				return false;
			}
			strncpy(pub_topic, argv[i + 1], sizeof(pub_topic));
			i++;
		}
		else if (!strcmp(argv[i], "-sub")) {
			if (i + 1 == argc) {
				showCmdLine();
				return false;
			}
			strncpy(sub_topic, argv[i + 1], sizeof(sub_topic));
			i++;
		}
		else if (!strcmp(argv[i], "-vin")) {
			if (i + 1 == argc) {
				showCmdLine();
				return false;
			}
			strncpy(vin, argv[i + 1], sizeof(vin));
			i++;
		}
		else if (!strcmp(argv[i], "-imei")) {
			if (i + 1 == argc) {
				showCmdLine();
				return false;
			}
			strncpy(imei, argv[i + 1], sizeof(imei));
			i++;
		}
		else if (!strcmp(argv[i], "-iccid")) {
			if (i + 1 == argc) {
				showCmdLine();
				return false;
			}
			strncpy(iccid, argv[i + 1], sizeof(iccid));
			i++;
		}
		else if (!strcmp(argv[i], "-tsn")) {
			if (i + 1 == argc) {
				showCmdLine();
				return false;
			}
			strncpy(tsn, argv[i + 1], sizeof(tsn));
			i++;
		}
		else if (!strcmp(argv[i], "-gps")) {
			if (i + 1 == argc) {
				showCmdLine();
				return false;
			}
			if (!strcmp(argv[i + 1], "on")) {
				gpsTaskAtStartup = true;
			}
			else if (!strcmp(argv[i + 1], "off")) {
				gpsTaskAtStartup = false;
			}
			else {
				showCmdLine();
				return false;
			}
			i++;
		}
		else if (!strcmp(argv[i], "-dto")) {
			if (i + 1 == argc) {
				showCmdLine();
				return false;
			}
			doorActivationTimeOut = atoi(argv[i + 1]);
			i++;
		}
		else if (!strcmp(argv[i], "-channels")) {
			startChannel = true;
			i++;
		}
		else {
			showCmdLine();
			return false;
		}
	}
#if BC_TARGET_LINUX == BC_TARGET
	isServer = false;
#else
	isServer = false;
#endif
	char topic[256];
	memset(topic, 0, 256);
	strncpy(topic, "mqtt/vehicle/", sizeof(topic));
	strcat(topic, vin);
	if (isServer) {
		strncpy(pub_topic, topic, sizeof(pub_topic));
		strncpy(sub_topic, "mqtt/server", sizeof(sub_topic));
		//strncpy(clientid, "serverid", sizeof(clientid));
	}
	else {
		strncpy(pub_topic, "mqtt/server", sizeof(pub_topic));
		strncpy(sub_topic, topic, sizeof(sub_topic));
		//#if BC_TARGET_LINUX == BC_TARGET
		//			strncpy(clientid, "clientidl", sizeof(clientid));
		//#else
		//			strncpy(clientid, "clientidw", sizeof(clientid));
		//#endif
	}
	char tmps[255];
	memcpy(tmps, vin, sizeof(vin));
	tmps[sizeof(vin)] = 0;
	LOG_I("vin: %s", tmps);

	memcpy(tmps, tsn, sizeof(tsn));
	tmps[sizeof(tsn)] = 0;
	LOG_I("tsn: %s", tmps);

	memcpy(tmps, imei, sizeof(imei));
	tmps[sizeof(imei)] = 0;
	LOG_I("imei: %s", tmps);

	memcpy(tmps, iccid, sizeof(iccid));
	tmps[sizeof(iccid)] = 0;
	LOG_I("tmps: %s", tmps);
	return true;
}

