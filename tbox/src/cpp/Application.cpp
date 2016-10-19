#include "../inc/Application.h"
#include "../tasks/RemoteUnlockTask.h"
#include "../tasks/VehicleAuthTask.h"
#include "../test/RemoteUnlockTest.h"

static Application* g_inst;
Application& Application::getInstance()
{
	return *g_inst;
}

void Application::init(int argc, char** argv)
{
	g_inst = this;
	LOG_I("Application::init(%d)", argc);
	DebugCode( for (int i = 0; i < argc; ++i) {
		LOG_I("    %s", argv[i]);
	})
	config.parse(argc, argv);
	mqtt.setConfig(getConfig());
	//launch thread to do branch task
	Thread::startThread(this);
}

bool Application::startTask(Task* task,bool runAsThread)
{
	if (runAsThread) {
		tasksWorking.in(task);
		Thread::startThread(task);
		return true;
	}
	if (tasksWaiting.in(task)) {
		ThreadEvent::PostResult pr = taskEvent.post();
		if (ThreadEvent::PostOk == pr) {
			LOG_I("startTask(%d,%d)", task->getApplicationId(), task->getSessionId());
			return true;
		}
		else {
			LOG_E("post event failed %d", pr);
			return false;
		}
	}
	else {
		LOG_E("Application::startTask() taskQueue.in() failed ");
		return false;
	}
}

void Application::loop()
{
	onDebugCommand("connMqtt");
	loopID = Thread::getCurrentThreadId();
	while (true) {
		LOG_V("Application loop...");
		ThreadEvent::WaitResult wr = appEvent.wait(500);
		if (wr == ThreadEvent::EventOk) {
			while (!appEventQueue.isEmpty()) {
				AppEvent::e e;
				u32 param1;
				u32 param2;
				void* data;
				if (appEventQueue.out(e, param1, param2, data)) {
					onEvent(e, param1,param2,data);
				}
				else {
					LOG_E("task should no be null,something wrong");
					break;
				}
			}
		}
		else if (wr == ThreadEvent::TimeOut) {

		}
		else {
			LOG_E("wrong wait result %d", wr);
			break;
		}
	}
}

bool Application::onDebugCommand(char* cmd)
{
	LOG_P("%s\r\n", cmd);
	if (!strcmp(cmd, "unlock")) {

		return true;
	}
	if (!strcmp(cmd, "lock")) {
		PostEvent(AppEvent::AddTask, 0, 0, bc_new RemoteUnlockTask(1, 2, 0));
		return true;
	}
	if (!strcmp(cmd, "connMqtt")) {
		mqtt.reqConnect(config.mqttServer, config.pub_topic, 0,config.keepAliveInterval,config.clientid);
		return true;
	}
	if (!strcmp(cmd, "discMqtt")) {
		mqtt.reqDisconnect();
		return true;
	}
	if (!strcmp(cmd, "reqUnlock")) {
		PostEvent(AppEvent::AddTask, 0, 0, bc_new RemoteUnlockTest(1, 1));
		return true;
	}
	if (mqtt.onDebugCommand(cmd))return true;
	return false;
}

bool Application::connectServer()
{
	onEvent(AppEvent::NetConnected, 0,0,0);
	return true;
}

void Application::disconnectServer()
{
	onEvent(AppEvent::NetDisconnected, 0,0,0);
}

void Application::run()
{
	while (true) {
		LOG_V("tasks run...");
		ThreadEvent::WaitResult wr = taskEvent.wait(500);
		if (wr == ThreadEvent::EventOk) {
			while (!tasksWaiting.isEmpty()) {
				Task* task = tasksWaiting.out();
				if (task != NULL) {
					tasksWorking.in(task);
					if (!task->isAsync) {
						task->run();
					}
					else {
						Thread::startThread(task);
					}
				}
				else {
					LOG_E("task should no be null,something wrong");
					break;
				}
			}
		}
		else if (wr == ThreadEvent::TimeOut) {

		}
		else {
			LOG_E("wrong wait result %d", wr);
			break;
		}
	}
}

void Application::onEvent(AppEvent::e e, u32 param1, u32 param2, void* data)
{
	switch (e)
	{
	case AppEvent::AddTask:
		LOG_A(data, "should not be null");
		startTask((Task*)data, true);
		break;
	case AppEvent::AbortTask:
		tasksWaiting.abortTask(param1);
		tasksWorking.abortTask(param1);
		break;
	case AppEvent::DelTask:
		LOG_A(data,"should not be null");
		tasksWorking.out((Task*)data);
		bc_del(Task*)data;
		break;
	case AppEvent::NetConnected:
		onNetConnected();
		break;
	case AppEvent::NetDisconnected:
		onNetDisconnected();
		break;
	case AppEvent::MqttStateChanged:
		onMqttEvent(param1,param2,data);
		break;
	case AppEvent::AutoStateChanged:
		break;
	case AppEvent::SensorEvent:
		break;
	default:
		break;
	}
	broadcastEvent(e, param1, param2, data);
}

Config *Application::getConfig(void)
{
	return &config;
}

bool Application::postAppEvent(AppEvent::e e, u32 param1, u32 param2, void* data)
{
	bool ret = appEventQueue.in(e, param1,param2, data);
	if (!ret && 
		Thread::getCurrentThreadId() != loopID) {
		while(appEventQueue.in(e, param1, param2, data)){}
		ret = true;
	}
	if (ret) {
		appEvent.post();
	}
	return ret;
}

void Application::broadcastEvent(AppEvent::e e, u32 param1, u32 param2, void* data)
{
	Task* t = tasksWorking.getNextTask(NULL);
	while (t) {
		t->onEvent(e, param1, param2, data);
		t = tasksWorking.getNextTask(t);
	}
}

Task* Application::findTask(u32 applicationId)
{
	return tasksWorking.findTask(applicationId);
}

void Application::onMqttEvent(u32 param1, u32 param2, void* data)
{
	LOG_I("onMqttEvent(%d,%d,%p)",param1,param2,data);
	if (param2 == MqttClient::Subscribed) {
		if (!config.isServer) { 
			startTask(bc_new VehicleAuthTask(), false); 
		}
	}
}

void Application::onNetConnected()
{
	LOG_I("onNetConnected");
	mqtt.reqConnect(config.mqttServer, config.sub_topic,0,config.keepAliveInterval,config.clientid);
}

void Application::onNetDisconnected()
{
	LOG_I("onNetDisconnected");
	mqtt.reqDisconnect();
}
