#ifndef MQTT_GUARD_Application_h__
#define MQTT_GUARD_Application_h__

#include "./Config.h"
#include "./TaskQueue.h"
#include "./TaskList.h"
#include "./State.h"
#include "./Mqtt.h"
#include "./Event.h"
#include "./Vehicle.h"

class Application : public Thread
{
	friend bool PostEvent(AppEvent,u32,u32,void*);
public:
	static Application& getInstance();
	Config& getConfig(void);
	Vehicle& getVehicle(void);
	void init(int argc, char** argv);
	//main loop process for app event
	void loop();
	bool postAppEvent(AppEvent::Type e, u32 param1, u32 param2, void* data);
	//debug interface
	bool onDebugCommand(char* cmd);
public:
	bool connectServer();
	void disconnectServer();
	Task* findTask(u32 applicationId);
protected:
	//thread running for tasks
	virtual void run()OVERRIDE;
private:
	bool startTask(Task* task, bool runAsThread);
	void onEvent(AppEvent::Type e, u32 param1, u32 param2, void* data);
	void broadcastEvent(AppEvent::Type e, u32 param1, u32 param2, void* data);
protected:
	void onMqttEvent(u32 param1, u32 param2, void* data);
	void onAutoStateChanged(u32 param1, u32 param2, void* data);
	void onNetStateChanged(u32 param);
public:
	Vehicle     vehicle;		//vehicle states
	Config      config;			//setting and configuration
	TaskQueue   tasksWaiting;	//tasks that waiting for launching
	TaskList    tasksWorking;	//tasks that already started
	ThreadEvent taskEvent;		//event trigger for tasks
	ThreadEvent appEvent;		//event for application
	EventQueue  appEventQueue;	//event message for application attached with appEvent
	MqttClient  mqtt;			//mqtt protocol for receiving and sending message
	Thread::ID  loopID;			//current thread id
};



#endif // GUARD_Application_h__
