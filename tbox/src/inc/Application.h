#ifndef MQTT_GUARD_Application_h__
#define MQTT_GUARD_Application_h__

#include "./Message.h"
#include "./Config.h"
#include "./TaskQueue.h"
#include "./TaskList.h"
#include "./State.h"
#include "./Mqtt.h"
#include "./Event.h"

class Application : public Thread
{
public:
	void init(int argc, char** argv);
	bool startTask(Task* task,bool runAsThread);
	//main loop process for event
	void loop();
	void onCommand(char* cmd);
protected:
	//thread running for tasks
	virtual void run()override;
private:
	void onEvent(AppEvent type,void* data,int len);
protected:
	void onNetConnected();
	void onNetDisconnected();
public:
	Config      config;
	TaskQueue   tasksWaiting;
	TaskList    tasksWorking;
	ThreadEvent taskEvent;
	ThreadEvent appEvent;
	MqttHandler mqtt;
};



#endif // GUARD_Application_h__
