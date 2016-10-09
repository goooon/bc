#include "../inc/Application.h"

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
		LOG_I("Application loop...");
		auto wr = appEvent.wait(500);
		if (wr == ThreadEvent::EventOk) {
			LOG_I("app event");
		}
		else if (wr == ThreadEvent::TimeOut) {
		}
		else {
			LOG_E("wrong wait result %d", wr);
			break;
		}
	}
}

void Application::onCommand(char* cmd)
{
	LOG_P(cmd);
}

bool Application::connectServer()
{
	onEvent(NetConnected, 0, 0);
	return true;
}

void Application::disconnectServer()
{
	onEvent(NetDisconnected, 0, 0);
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

void Application::onEvent(AppEvent type, void* data, int len)
{
	switch (type)
	{
	case NetConnected:
		onNetConnected();
		break;
	case NetDisconnected:
		onNetDisconnected();
		break;
	case ServerConnected:
		onServerConnected();
		break;
	case ServerDisconnected:
		onServerDisconnected();
		break;
	default:
		break;
	}
}

void Application::onServerConnected()
{
	LOG_I("onServerConnected");
}

void Application::onServerDisconnected()
{
	LOG_I("onServerDisconnected");
}

void Application::onNetConnected()
{
	mqtt.reqConnect("tcp://m2m.eclipse.org:1883", config.topics,0);
}

void Application::onNetDisconnected()
{
	mqtt.reqDisconnect();
}
