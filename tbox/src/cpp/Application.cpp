#include "../inc/Application.h"
#include "../tasks/VKeyActiveTask.h"
#include "../tasks/VKeyDeactiveTask.h"
#include "../tasks/VKeyIgnitionTask.h"
#include "../tasks/VehicleAuthTask.h"
#include "../tasks//StateUploadTask.h"
#include "../test/ActiveTest.h"
#include "../tasks/TaskTable.h"
static Application* g_inst;
Application& Application::getInstance()
{
	return *g_inst;
}

bool Application::init(int argc, char** argv)
{
	g_inst = this;
	LOG_I("Application::init(%d)", argc);
	DebugCode( for (int i = 0; i < argc; ++i) {
		LOG_I("    %s", argv[i]);
	})
		if (!config.parse(argc, argv))return false;
	//launch thread to do branch task
	Thread::startThread(this);
	mqtt.onDebugCommand("PROTOCOL");
	return true;
}

bool Application::startTask(Task* task,bool runAsThread)
{
	if (task == NULL) {
		LOG_E("startTask is null");
		return false;
	}
	if (runAsThread) {
		tasksWorking.in(task);
		Thread::startThread(task);
		return true;
	}
	if (tasksWaiting.in(task)) {
		ThreadEvent::PostResult pr = taskEvent.post();
		if (ThreadEvent::PostOk == pr) {
			LOG_I("startTask(%d,%lld)", task->getApplicationId(), task->getSequenceId());
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
	LOG_V("Application loop...");
	//if(!config.isServer)onDebugCommand("connMqtt");
	loopID = Thread::getCurrentThreadId();
	Timestamp prev;
	Timestamp now;
	while (true) {
		ThreadEvent::WaitResult wr = appEvent.wait(500);
		if (wr == ThreadEvent::EventOk) {
			while (!appEventQueue.isEmpty()) {
				AppEvent::Type e;
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
		now.update();
		if (now - prev > 500) {
			schedule.triger(now);
			prev = now;
		}
	}
}

bool Application::onDebugCommand(const char* cmd)
{
	if (!strcmp(cmd, "reqDeact")) {
		Task* p = bc_new VKeyDeavtiveTask();
		p->handleDebug();
		PostEvent(AppEvent::InsertTask, 0, 0, p);
		return true;
	}
	if (!strcmp(cmd, "reqActive")) {
		Task* p = bc_new VKeyActiveTask();
		p->handleDebug();
		PostEvent(AppEvent::InsertTask, 0, 0, p);
		return true;
	}
	if (!strcmp(cmd, "reqIgnit")){
		Task* p = bc_new VKeyIgnitionTask();
		p->handleDebug();
		PostEvent(AppEvent::InsertTask, 0, 0, p);
		return true;
	}
	if (!strcmp(cmd, "connMqtt")) {
		mqtt.reqConnect(config.mqttServerIp, config.sub_topic, 0,config.keepAliveInterval,config.clientid);
		return true;
	}
	if (!strcmp(cmd, "discMqtt")) {
		mqtt.reqDisconnect();
		return true;
	}
	//if (!strcmp(cmd, "reqUnlock")) {
	//	Task* t = bc_new ActiveTest();
	//	::PostEvent(AppEvent::AbortTasks, t->getApplicationId(), 0, 0);
	//	::PostEvent(AppEvent::InsertTask, 0, 0, t);
	//	return true;
	//}
	//
	if (mqtt.onDebugCommand(cmd))return true;
	return false;
}

bool Application::connectServer()
{
	onEvent(AppEvent::NetStateChanged, 1,0,0);
	return true;
}

void Application::disconnectServer()
{
	onEvent(AppEvent::NetStateChanged, 0,0,0);
}

void Application::run()
{
	while (true) {
		ThreadEvent::WaitResult wr = taskEvent.wait(1000);
		//LOG_I("tasks run...");
		if (wr == ThreadEvent::EventOk) {
			while (!tasksWaiting.isEmpty()) {
				Task* task = tasksWaiting.out();
				if (task != NULLPTR) {
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

void Application::onEvent(AppEvent::Type e, u32 param1, u32 param2, void* data)
{
	switch (e)
	{
	case AppEvent::InsertTask:
		LOG_A(data, "should not be null");
		startTask((Task*)data, true);
		break;
	case AppEvent::AbortTasks:
		tasksWaiting.abortTask(param1);
		tasksWorking.abortTask(param1);
		break;
	case AppEvent::RemoveTask:
		LOG_A(data,"should not be null");
		tasksWorking.out((Task*)data);
		bc_del((Task*)data);
		break;
	case AppEvent::NetStateChanged:
		onNetStateChanged(param1);
		broadcastEvent(AppEvent::NetStateChanged, param1, param2, data);
		break;
	case AppEvent::MqttStateChanged:
		onMqttStateChanged(param1,param2,data);
		broadcastEvent(AppEvent::MqttStateChanged, param1, param2, data);
		break;
	case AppEvent::AutoStateChanged:
		onAutoStateChanged(param1, param2, data);
		broadcastEvent(AppEvent::AutoStateChanged, param1, param2, data);
		break;
	case AppEvent::SensorEvent:
		break;
	case AppEvent::AutoEvent:
		Vehicle::getInstance().onEvent(param1, param2, 0);
		broadcastEvent(e, param1, param2, data);
		break;
	case AppEvent::InsertSchedule:
		schedule.insert(Timestamp(param1, param2), (Task*)data);
		break;
	case AppEvent::RemoveSchedule:
		if(data)schedule.remove((Task*)data);
		else schedule.remove(param1);
		break;
    case AppEvent::UpdateSchedule:
		schedule.update(Timestamp(param1, param2), *((int*)(&data)));
		break;
	default:
		break;
	}
}

bool Application::postAppEvent(AppEvent::Type e, u32 param1, u32 param2, void* data)
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

Config& Application::getConfig(void)
{
	return config;
}

Vehicle& Application::getVehicle(void)
{
	return vehicle;
}

Schedule& Application::getSchedule(void)
{
	return schedule;
}

PackageQueue& Application::getPackageQueue(void)
{
	return pkgQueue;
}

//
//Schedule& Application::getSchedule(void)
//{
//	return schedule;
//}

void Application::broadcastEvent(AppEvent::Type e, u32 param1, u32 param2, void* data)
{
	Task* t = tasksWorking.getNextTask(NULLPTR);
	while (t) {
		t->onEvent(e, param1, param2, data);
		t = tasksWorking.getNextTask(t);
	}
}

Task* Application::findTask(u32 applicationId)
{
	return tasksWorking.findTask(applicationId);
}

void Application::onMqttStateChanged(u32 param1, u32 param2, void* data)
{
	LOG_I("onMqttStateChanged(%d,%d,%p)",param1,param2,data);
	if (param2 == MqttClient::Subscribed) {
		if (!config.isServer) { 
			startTask(TaskCreate(APPID_AUTHENTICATION,0), false);
		}
	}
}

void Application::onAutoStateChanged(u32 param1, u32 param2, void* data)
{
	LOG_I("onAutoStateChanged(%d,%d,%p)", param1, param2, data);
	Vehicle::State prev = (Vehicle::State)param1;
	Vehicle::State next = (Vehicle::State)param2;
	if (prev == Vehicle::Ignited && next == Vehicle::ReadyToIgnit)
	{
		Timestamp ts;
		ts.update(Config::getInstance().getStateUploadExpireTime());
		schedule.remove(APPID_STATE_UPLOADING_NTF);
		schedule.insert(ts, bc_new StateUploadTask_NTF());
	}
}

void Application::onNetStateChanged(u32 param)
{
	if (param == 1) {
		LOG_I("onNetConnected");
		mqtt.reqConnect(config.mqttServerIp, config.sub_topic, 0, config.keepAliveInterval, config.clientid);
	}
	else if (param == 0) {
		LOG_I("onNetDisconnected");
		mqtt.reqDisconnect();
	}
	else {
		LOG_E("unknown state %d", param);
	}
}
