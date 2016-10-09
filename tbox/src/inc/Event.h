#ifndef MQTT_GUARD_Event_h__
#define MQTT_GUARD_Event_h__

#include "./dep.h"

enum AppEvent
{
	NetConnected,
	ServerConnected,
	ServerDisconnected,
	NetDisconnected
};
#endif // MQTT_GUARD_Event_h__
