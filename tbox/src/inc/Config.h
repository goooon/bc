#ifndef MQTT_GUARD_Config_h__
#define MQTT_GUARD_Config_h__

#include <string.h>
#include "../../../fundation/src/inc/fundation.h"

class Config
{
public:
	static Config& getInstance();
public:
	bool parse(int argc, char** argv) {
		strncpy(mqttServer, "139.219.238.66:1883", sizeof(mqttServer));
		keepAliveInterval = 20;
		isServer = false;
		if (isServer) {
			strncpy(pub_topic, "beecloud-clientid", sizeof(pub_topic));
			strncpy(sub_topic, "beecloud-server", sizeof(sub_topic));
			strncpy(clientid, "serverid", sizeof(clientid));
		}
		else {
			strncpy(pub_topic, "beecloud-server", sizeof(pub_topic));
			strncpy(sub_topic, "beecloud-clientid", sizeof(sub_topic));
			strncpy(clientid, "clientid", sizeof(clientid));
		}
		return true;
	}
private:
public:
	u32 ip;
	u16 port;
	int keepAliveInterval;
	bool isServer;
	char pub_topic[32];
	char sub_topic[32];
	char mqttServer[256];
	char clientid[32];
};
#endif // GUARD_Config_h__
