#ifndef MQTT_GUARD_Config_h__
#define MQTT_GUARD_Config_h__

#include <string.h>
#include "./dep.h"
#include "../../../fundation/src/inc/fundation.h"

class Config
{
public:
	static Config& getInstance();
	void showCmdLine() {
		printf("-ip xx.xx.xx.xx:port\r\n");
		printf("-dto doorActivationTimeOut");
		printf("-pub publish_topic\r\n");
		printf("-sub subscription\r\n");
		printf("-cid clientid\r\n");
		printf("-vin vehicle vin\r\n");
	}
	void setDoorActivationTimeOut(u32 to) {
		doorActivationTimeOut = to;
	}
	void setIgntActivationTimeOut(u32 to){
		igntActivationTimeOut = to;
	}
	void setStateUploadExpireTime(u32 to) {
		stateUploadExpireTime = to;
	}
	void setGpsIntervalInDriving(u32 it) {
		gpsIntervalDriving = it;
	}
	void setGpsIntervalInStation(u32 it) {
		gpsIntervalStation = it;
	}
	void setAbnormalMovingDuration(u32 it) {
		gpsIntervalAbormal = it;
	}

	u32 getDoorActivationTimeOut() {
		return doorActivationTimeOut;
	}
	u32 getIgntActivationTimeOut() {
		return igntActivationTimeOut;
	}
	u32 getStateUploadExpireTime() {
		return stateUploadExpireTime;
	}
	u32 getMqttReConnInterval() {
		return mqttReConnInterval;
	}
	u32 getAuthRetryInterval() {
		return authRetryInterval;
	}
	u32 getGPSQueueSize() {
		return gpsQueueSize;
	}
	bool isGpsTaskAtStartup() {
		return gpsTaskAtStartup;
	}
	
	u32 getGpsInterval();
	u32  getAuthToken() { return authToken; }
	void setAuthToken(u32 t) { authToken = t; }
public:
	bool parse(int argc, char** argv) {
		memset(pub_topic, 0, sizeof(pub_topic));
		memset(sub_topic, 0, sizeof(sub_topic));
		memset(mqttServerIp, 0, sizeof(mqttServerIp));
		//memset(clientid, 0, sizeof(clientid));
		memset(vin, 0, sizeof(vin));

		gpsTaskAtStartup = 0;
		gpsQueueSize = 4096;
		doorActivationTimeOut = 5000;
		igntActivationTimeOut = 5000;
		mqttReConnInterval = 5000;
		authRetryInterval = 5000;
		gpsIntervalAbormal = 30000;
		gpsIntervalStation = 1000 * 60 * 5;
		gpsIntervalDriving = 1000 * 30;
		stateUploadExpireTime = 2 * 1000; //2 min
		strncpy(mqttServerIp, "10.28.4.40:1884", sizeof(mqttServerIp));//main server
		 //strncpy(mqttServer, "139.219.238.66:1883", sizeof(mqttServer));//test server
		 //strncpy(mqttServer, "10.28.248.71:1883", sizeof(mqttServer));//guzhibin

		strncpy(vin, "VIN67423921", sizeof(vin));//main server
		keepAliveInterval = 20;

	//////////////////////////////////////////////////////////////////////////
		for (int i = 1; i < argc; i++) {
			if (!strcmp(argv[i], "-ip")) {
				if (i + 1 == argc) {
					showCmdLine();
					return false;
				}
				strncpy(mqttServerIp, argv[i+1], sizeof(mqttServerIp));//main server
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
			/*else if (!strcmp(argv[i], "-cid")) {
				if (i + 1 == argc) {
					showCmdLine();
					return false;
				}
				strncpy(clientid, argv[i + 1], sizeof(clientid));
				i++;
			}*/
			else if (!strcmp(argv[i], "-dto")){
				if (i + 1 == argc) {
					showCmdLine();
					return false;
				}
				doorActivationTimeOut = atoi(argv[i + 1]);
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
		return true;
	}
	const char* getPublishTopic() { return pub_topic; }
private:
public:
	u32  ip;
	u16  port;
	int  keepAliveInterval;
	bool isServer;
	char pub_topic[64];
	char sub_topic[64];
	char mqttServerIp[256];
	char vin[17];	//utf-8 string
	u32  authToken;
	//////////////////////////////////////////////////////////////////////////
public:
	bool gpsTaskAtStartup;
	u32 gpsQueueSize;
	u32 mqttReConnInterval;
	u32 authRetryInterval;
	u32 gpsIntervalDriving;
	u32 gpsIntervalStation;
	u32 gpsIntervalAbormal;
	u32 doorActivationTimeOut;
	u32 igntActivationTimeOut;
	u32 stateUploadExpireTime;
};
#endif // GUARD_Config_h__
