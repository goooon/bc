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
	Thread::startThread(this);
}

bool Application::startTask(Task* task,bool runAsThread)
{
	if (runAsThread) {
		tasksWorking.in(task);
		task->refList = &tasksWorking;
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
			LOG_I("app event");
			while (!appEventQueue.isEmpty()) {
				AppEvent e;
				u32 param1;
				void* data;
				u32 param2;
				bool ok = appEventQueue.out(e, param1,param2,data);
				if (ok) {
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
		startTask(bc_new RemoteUnlockTask(1, 2, true),false);
		return true;
	}
	if (!strcmp(cmd, "lock")) {
		startTask(bc_new RemoteUnlockTask(1, 2, true), false);
		return true;
	}
	if (!strcmp(cmd, "connMqtt")) {
		mqtt.reqConnect(config.mqttServer, config.topics, 0);
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
		LOG_V("Application run...");
		auto wr = taskEvent.wait(500);
		if (wr == ThreadEvent::EventOk) {
			while (!tasksWaiting.isEmpty()) {
				Task* task = tasksWaiting.out();
				if (task != nullptr) {
					task->refList = &tasksWorking;
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
	case NetConnected:
		onNetConnected();
		break;
	case NetDisconnected:
		onNetDisconnected();
		break;
	case MqttEvent:
		onMqttEvent();
		break;
	default:
		break;
	}
}

bool Application::postAppEvent(AppEvent e, u32 param1, u32 param2, void* data)
{
	bool ret = appEventQueue.in(e, param1,param2, data);
	if (ret) {
		appEvent.post();
	}
	return ret;
}

void Application::onMqttEvent()
{
	LOG_I("onMqttEvent");
	
}

void Application::onNetConnected()
{
	LOG_I("onNetConnected");
	mqtt.reqConnect(config.mqttServer, config.topics,0);
}

void Application::onNetDisconnected()
{
	LOG_I("onNetDisconnected");
	mqtt.reqDisconnect();
}
