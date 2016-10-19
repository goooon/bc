#ifndef MQTT_GUARD_Application_h__
#define MQTT_GUARD_Application_h__

#include "./Message.h"
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
	void init(int argc, char** argv);
	
	//main loop process for app event
	void loop();
	bool onDebugCommand(char* cmd);
	bool postAppEvent(AppEvent::e e, u32 param1, u32 param2, void* data);
	Config *getConfig(void);
public:
	bool connectServer();
	void disconnectServer();
	Task* findTask(u32 applicationId);
protected:
	//thread running for tasks
	virtual void run()OVERRIDE;
private:
	bool startTask(Task* task, bool runAsThread);
	void onEvent(AppEvent::e e, u32 param1, u32 param2, void* data);
	void broadcastEvent(AppEvent::e e, u32 param1, u32 param2, void* data);
	
protected:
	void onMqttEvent(u32 param1, u32 param2, void* data);
	void onNetConnected();
	void onNetDisconnected();
public:
	Vehicle     vehicle;
	Config      config;
	TaskQueue   tasksWaiting;
	TaskList    tasksWorking;
	ThreadEvent taskEvent;
	ThreadEvent appEvent;
	EventQueue  appEventQueue;
	MqttClient  mqtt;
	Thread::ID  loopID;
};



#endif // GUARD_Application_h__
