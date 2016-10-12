#include "../inc/Application.h"
#include "../inc/RemoteUnlockTask.h"
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
		auto pr = taskEvent.post();
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
	while (true) {
		LOG_V("Application loop...");
		auto wr = appEvent.wait(500);
		if (wr == ThreadEvent::EventOk) {
			while (!appEventQueue.isEmpty()) {
				AppEvent e;
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
	LOG_P(cmd);LOG_P("\r\n");
	if (!strcmp(cmd, "unlock")) {

		return true;
	}
	if (!strcmp(cmd, "lock")) {
		PostEvent(AppEvent::AddTask, 0, 0, bc_new RemoteUnlockTask(1, 2, true));
		return true;
	}
	if (!strcmp(cmd, "connMqtt")) {
		mqtt.reqConnect(config.mqttServer, config.topics, 0,config.keepAliveInterval);
		return true;
	}
	if (!strcmp(cmd, "discMqtt")) {
		mqtt.reqDisconnect();
		return true;
	}
	if (mqtt.onDebugCommand(cmd))return true;
	return false;
}

bool Application::connectServer()
{
	onEvent(NetConnected, 0,0,0);
	return true;
}

void Application::disconnectServer()
{
	onEvent(NetDisconnected, 0,0,0);
}

void Application::run()
{
	while (true) {
		LOG_V("tasks run...");
		auto wr = taskEvent.wait(500);
		if (wr == ThreadEvent::EventOk) {
			while (!tasksWaiting.isEmpty()) {
				Task* task = tasksWaiting.out();
				if (task != nullptr) {
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

void Application::onEvent(AppEvent e, u32 param1, u32 param2, void* data)
{
	switch (e)
	{
	case AddTask:
		LOG_A(data, "should not be null");
		startTask((Task*)data, true);
		break;
	case DelTask:
		LOG_A(data,"should not be null");
		tasksWorking.out((Task*)data);
		bc_del(Task*)data;
		break;
	case NetConnected:
		onNetConnected();
		break;
	case NetDisconnected:
		onNetDisconnected();
		break;
	case MqttStateChanged:
		onMqttEvent(param1,param2,data);
		break;
	case AutoStateChanged:
		break;
	case SensorEvent:
		break;
	default:
		break;
	}
	broadcastEvent(e, param1, param2, data);
}

bool Application::postAppEvent(AppEvent e, u32 param1, u32 param2, void* data)
{
	bool ret = appEventQueue.in(e, param1,param2, data);
	if (ret) {
		appEvent.post();
	}
	return ret;
}

void Application::broadcastEvent(AppEvent e, u32 param1, u32 param2, void* data)
{
	Task* t = tasksWorking.getTask(nullptr);
	while (t) {
		t->onEvent(e, param1, param2, data);
		t = tasksWorking.getTask(t);
	}
}

void Application::onMqttEvent(u32 param1, u32 param2, void* data)
{
	LOG_I("onMqttEvent(%d,%d,0x%x)",param1,param2,data);
}

void Application::onNetConnected()
{
	LOG_I("onNetConnected");
	mqtt.reqConnect(config.mqttServer, config.topics,0,config.keepAliveInterval);
}

void Application::onNetDisconnected()
{
	LOG_I("onNetDisconnected");
	mqtt.reqDisconnect();
}
