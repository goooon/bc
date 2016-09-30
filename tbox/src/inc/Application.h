#ifndef MQTT_GUARD_Application_h__
#define MQTT_GUARD_Application_h__

#include "./Message.h"
#include "./Config.h"
#include "./TaskQueue.h"
#include "./State.h"
class Application
{
public:
	void init(int argc, char** argv);
	bool startTask(Task* task);
	void run();
public:
	Config config;
	TaskQueue taskQueue;
	ThreadEvent e;
};



#endif // GUARD_Application_h__
