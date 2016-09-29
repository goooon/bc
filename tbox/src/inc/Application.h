#ifndef MQTT_GUARD_Application_h__
#define MQTT_GUARD_Application_h__

#include "./Message.h"
#include "./Config.h"
#include "./TaskQueue.h"
class Application
{
public:
	void init(int argc, char** argv);
public:
	Config config;
	TaskQueue taskQueue;
};



#endif // GUARD_Application_h__
