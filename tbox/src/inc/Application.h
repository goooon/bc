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
	friend bool PostEvent(AppEvent e, u32 param, void* data, int len);
public:
	static Application& getInstance();
	void init(int argc, char** argv);
	bool startTask(Task* task,bool runAsThread);
	//main loop process for event
	void loop();
	bool onDebugCommand(char* cmd);
public:
	bool connectServer();
	void disconnectServer();
protected:
	//thread running for tasks
	virtual void run()override;
private:
	void onEvent(AppEvent type,u32 param,void* data,int len);
	bool setAppEvent(AppEvent type, u32 param, void* data, int len);
protected:
	void onServerConnected();
	void onServerDisconnected();
	void onNetConnected();
	void onNetDisconnected();
public:
	Config      config;
	TaskQueue   tasksWaiting;
	TaskList    tasksWorking;
	ThreadEvent taskEvent;
	ThreadEvent appEvent;
	EventQueue  appEventQueue;
	MqttHandler mqtt;
};



#endif // GUARD_Application_h__
