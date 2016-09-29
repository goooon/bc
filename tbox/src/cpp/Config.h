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
		return true;
	}
private:

};
#endif // GUARD_Config_h__
