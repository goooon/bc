#ifndef MQTT_GUARD_BCPSender_h__
#define MQTT_GUARD_BCPSender_h__

#include "./Message.h"

//beecloud protocol sender
class BCPSender
{
public:
	//
	bool initialize()
	{

	}
	bool connect();
	void disconnect();
	void finalize();
};

#endif // MQTT_GUARD_BCPSender_h__
