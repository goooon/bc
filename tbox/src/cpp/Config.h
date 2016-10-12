#ifndef MQTT_GUARD_Config_h__
#define MQTT_GUARD_Config_h__

#include "../../../fundation/src/inc/fundation.h"

class Config
{
public:
	static Config& getInstance() {
		static Config cfg;
		return cfg;
	}
public:
	bool parse(int argc, char** argv) {
		strncpy(mqttServer, "139.219.238.66:1883", sizeof(mqttServer));
		strncpy(topics, "topics", sizeof(topics));
		keepAliveInterval = 20;
		return true;
	}
private:
public:
	u32 ip;
	u16 port;
	int keepAliveInterval;
	char topics[32];
	char mqttServer[256];
};
#endif // GUARD_Config_h__
