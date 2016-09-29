#ifndef MQTT_GUARD_CmdParser_h__
#define MQTT_GUARD_CmdParser_h__
#include "../../../fundation/src/inc/fundation.h"
#include "./Config.h"

class CmdParser
{
private:
public:
	static Config& parse(int argc, char** argv) {
		return Config::getInstance();
	}
};

#endif // GUARD_CmdParser_h__
