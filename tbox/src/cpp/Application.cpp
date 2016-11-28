#include "../inc/Application.h"
#include "../tasks/VKeyActiveTask.h"
#include "../tasks/VKeyDeactiveTask.h"
#include "../tasks/VKeyIgnitionTask.h"
#include "../tasks/VehicleAuthTask.h"
#include "../tasks/StateUploadTask.h"
#include "../tasks/MqttConnTask.h"
#include "../test/ActiveTest.h"
#include "../tasks/TaskTable.h"
#include "../tasks/GpsUploadTask.h"
#include "../inc/channels.h"

#if BC_TARGET == BC_TARGET_ANDROID
#include "../android/android_vicp.h"
#endif

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
	
	if (!config.parse(argc, argv))
		return false;

	bcp_init();

	//launch thread to do branch task
	netConnected = false;
	Thread::startThread(this);
	mqtt.onDebugCommand("PROTOCOL");
	
	if (config.isGpsTaskAtStartup()) {
		LOG_I("start GPS task.");
		PostEvent(AppEvent::InsertTask, 0, 0, TaskCreate(APPID_GPS_UPLOADING_NTF_CONST,0));
	}
	if (config.isStartChannels()) {
		channels_init();
	}
#if BC_TARGET == BC_TARGET_ANDROID
	if (config.isStartOnAndroid()) {
		android_vicp_init();
	}
#endif

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
	if (!strcmp(cmd, "Active")) {
		PostEvent(AppEvent::AutoEvent, Vehicle::ActiveDoorResult, 1, 0);
		return true;
	}
	if (!strcmp(cmd, "DeActived")) {
		PostEvent(AppEvent::AutoEvent,Vehicle::DeactiveDoorResult, 1, 0);
		return true;
	}
	if (!strcmp(cmd, "reqDeact")) {
		Task* p = TaskCreate(APPID_VKEY_DEACTIVITION, 0);// bc_new VKeyDeavtiveTask();
		p->handleDebug();
		PostEvent(AppEvent::InsertTask, 0, 0, p);
		return true;
	}
	if (!strcmp(cmd, "reqActive")) {
		Task* p = TaskCreate(APPID_VKEY_ACTIVITION,0); //bc_new VKeyActiveTask();
		p->handleDebug();
		PostEvent(AppEvent::InsertTask, 0, 0, p);
		return true;
	}
	if (!strcmp(cmd, "reqIgnit")){
		Task* p = TaskCreate(APPID_VKEY_IGNITION, 0); //bc_new VKeyReadyToIgnitionTask();
		p->handleDebug();
		PostEvent(AppEvent::InsertTask, 0, 0, p);
		return true;
	}
	if (!strcmp(cmd, "reqReady")) {
		Task* p = TaskCreate(APPID_VKEY_IGNITION, 0); //bc_new VKeyReadyToIgnitionTask();
		p->handleDebug();
		PostEvent(AppEvent::InsertTask, 0, 0, p);
		return true;
	}
	if (!strcmp(cmd, "AbnMove")) {
		PostEvent(AppEvent::AutoEvent, Vehicle::AbormalMove, 0, 0);
		return true;
	}
	if (!strcmp(cmd, "NorMove")) {
		PostEvent(AppEvent::AutoEvent, Vehicle::NormalMove, 0, 0);
		return true;
	}
	if (!strcmp(cmd, "connMqtt")) {
		mqtt.reqConnect(config.mqttServerIp, config.sub_topic, 0,config.keepAliveInterval,config.vin);
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

bool Application::isNetConnected()
{
	return netConnected;
}

bool Application::isMqttConnected()
{
	return mqtt.isConnected();
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
		if (data != NULL) {
			startTask((Task*)data, true);
		}
		else {
			startTask(TaskCreate(param1, 0), true);
		}
		break;
	case AppEvent::AbortTasks:
		tasksWaiting.abortTask(param1);
		tasksWorking.abortTask(param1);
		break;
	case AppEvent::RemoveTask:
		if (data == NULL) {
			LOG_E("should not be null");
		}
		else {
			tasksWorking.out((Task*)data);
			bc_del((Task*)data);
		}
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
		if (param1 == Vehicle::AuthIdentity && param2 == Vehicle::Authed) {
			PostEvent(AppEvent::InsertTask, APPID_PACKAGE_QUEUE, 0, 0);
		}
		else if (param1 == Vehicle::DeactiveDoorResult && param2) {
			if (!Vehicle::getInstance().isIgnited()) {
				Timestamp ts;
				ts.update(Config::getInstance().getStateUploadExpireTime());
				schedule.update(ts, APPID_STATE_UNIGNITION_NTF);
			}
		}
		else if (param1 == Vehicle::UnIgnt) {
			startTask(TaskCreate(APPID_STATE_UNIGNITION_NTF, 0), true);
		}
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

CanBus& Application::getCanBus(void)
{
	return canBus;
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
	LOG_I("onMqttStateChanged(%d,%d,0x%p)",param1,param2,data);
	if (param2 == MqttClient::Subscribed) {
		if (!config.isServer) { 
			startTask(TaskCreate(APPID_AUTHENTICATION,0), false);
		}
	}
	else if (param2 == MqttClient::Disconnected) {
		if (!config.isServer){
			if (netConnected) {
				Timestamp ts;
				ts.update(config.getMqttReConnInterval());
				if (param1 != MqttClient::Connecting) {
					PostEvent(AppEvent::InsertSchedule, ts.h, ts.l, bc_new MqttConnTask());
				}
				PostEvent(AppEvent::AutoEvent, Vehicle::AuthIdentity, Vehicle::Unauthed, 0);
			}
			else {
				
			}
		}
	}
}

void Application::onAutoStateChanged(u32 param1, u32 param2, void* data)
{
	LOG_I("onAutoStateChanged(%d,%d,%p)", param1, param2, data);
	Vehicle::State prev = (Vehicle::State)param1;
	Vehicle::State next = (Vehicle::State)param2;
	if (prev == Vehicle::Authing && next == Vehicle::Authed) {
		startTask(TaskCreate(APPID_ACQUIRE_CONFIG, 0), true);
	}
	else if (prev == Vehicle::Ignited && next == Vehicle::ReadyToIgnit){
		Timestamp ts;
		ts.update(Config::getInstance().getStateUploadExpireTime());
		schedule.replace(ts, TaskCreate(APPID_STATE_UNIGNITION_DELAY_NTF,0));
//		startTask(TaskCreate(APPID_VKEY_UNIGNITION, 0), true);
	}
	else if (prev == Vehicle::ReadyToIgnit && next == Vehicle::Ignited) {
		startTask(TaskCreate(APPID_STATE_IGNITION, 0), true);
	}
}

void Application::onNetStateChanged(u32 param)
{
	if (param == 1) {
		LOG_I("onNetConnected");
		netConnected = true;
		reConnectMqtt();
	}
	else if (param == 0) {
		LOG_I("onNetDisconnected");
		netConnected = false;
		mqtt.reqDisconnect();
		PostEvent(AppEvent::AutoEvent, Vehicle::AuthIdentity, Vehicle::Unauthed, 0);
		PostEvent(AppEvent::AbortTasks, APPID_MQTT_CONNECT, 0, 0);
		PostEvent(AppEvent::RemoveSchedule, APPID_MQTT_CONNECT, 0, 0);
	}
	else {
		LOG_E("unknown state %d", param);
	}
}

void Application::reConnectMqtt()
{
	LOG_I("reConnectMqtt()");
	mqtt.reqConnect(config.mqttServerIp, config.sub_topic, 0, config.keepAliveInterval, config.vin);
}
