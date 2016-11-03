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
	}
	void setDoorActivationTimeOut(u32 to) {
		doorActivationTimeOut = to;
	}
	u32 getDoorActivationTimeOut() {
		return doorActivationTimeOut;
	}
public:
	bool parse(int argc, char** argv) {
		doorActivationTimeOut = 20;
		strncpy(mqttServer, "10.28.4.40:1884", sizeof(mqttServer));//main server
		 //strncpy(mqttServer, "139.219.238.66:1883", sizeof(mqttServer));//test server
		 //strncpy(mqttServer, "10.28.248.71:1883", sizeof(mqttServer));//guzhibin

		keepAliveInterval = 20;
#if BC_TARGET_LINUX == BC_TARGET
		isServer = false;
#else
		isServer = true;
#endif
		if (isServer) {
			strncpy(pub_topic, "mqtt/vehicle/VIN67423921", sizeof(pub_topic));
			strncpy(sub_topic, "mqtt/server", sizeof(sub_topic));
			strncpy(clientid, "serverid", sizeof(clientid));
		}
		else {
			strncpy(pub_topic, "mqtt/server", sizeof(pub_topic));
			strncpy(sub_topic, "mqtt/vehicle/VIN67423921", sizeof(sub_topic));
#if BC_TARGET_LINUX == BC_TARGET
			strncpy(clientid, "clientidl", sizeof(clientid));
#else
			strncpy(clientid, "clientidw", sizeof(clientid));
#endif
		}
	//////////////////////////////////////////////////////////////////////////
		for (int i = 1; i < argc; i++) {
			if (!strcmp(argv[i], "-ip")) {
				if (i + 1 == argc) {
					showCmdLine();
					return false;
				}
				strncpy(mqttServer, argv[i+1], sizeof(mqttServer));//main server
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
			else if (!strcmp(argv[i], "-cid")) {
				if (i + 1 == argc) {
					showCmdLine();
					return false;
				}
				strncpy(clientid, argv[i + 1], sizeof(clientid));
				i++;
			}
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
		return true;
	}
private:
public:
	u32  ip;
	u16  port;
	int  keepAliveInterval;
	bool isServer;
	char pub_topic[64];
	char sub_topic[64];
	char mqttServer[256];
	char clientid[64];
	//////////////////////////////////////////////////////////////////////////
private:
	u32 doorActivationTimeOut;
};
#endif // GUARD_Config_h__
