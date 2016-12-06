#include "../inc/dep.h"
#include "../inc/Application.h"
#include "../inc/Event.h"
#include "../inc/Sensor.h"
#include "../tasks/TaskTable.h"
#include "../test/ActiveTest.h"
#include "../tasks/StateUploadTask.h"
#include <errno.h>
unsigned int last_error(void)
{
#if defined(WIN32) || defined(WIN64)
	return (unsigned int)GetLastError();
#else
	return errno;
#endif
}

bool onClientTest(char* cmd) {
	if (!strcmp("auth", cmd)) {
		PostEvent(AppEvent::InsertTask, 0, 0, TaskCreate(APPID_AUTHENTICATION, STEPID_AUTHENTICATION, 0));
		return true;
	}
	if (!strcmp(cmd, "ntfState")) {
		::PostEvent(AppEvent::InsertTask, APPID_STATE_UNIGNITION_DELAY_NTF, STEPID_STATE_UNIGNITION_DELAY_NTF, 0);
		return true;
	}
	if (!strcmp(cmd, "ActiveTest")) {
		::Task* t = bc_new ActiveTest();
		::PostEvent(AppEvent::InsertTask, 0, 0, t);
		return true;
	}
	if (!strcmp(cmd, "DeactiveTest")) {
		::Task* t = bc_new ActiveTest();
		::PostEvent(AppEvent::InsertTask, 0, 0, t);
		return true;
	}
#define OPER_DOOR(op,cc,door_pos,id,bl) if (!strcmp(cc, cmd)) { \
		PostEvent(AppEvent::AutoEvent, Vehicle::op, id, 0); \
		return true; \
	}
	//OPER_DOOR(DoorOpened, "openDoor0", door.lh_front, 0, true);
	if (!strcmp("openDoor0", cmd)) {
		PostEvent(AppEvent::AutoEvent, Vehicle::DoorOpened, 4, 0);
		PostEvent(AppEvent::AutoEvent, Vehicle::DoorOpened, 0, 0);
		return true;
	}
	//OPER_DOOR(DoorOpened, "openDoor1", door.rh_front, 1, true);
	if (!strcmp("openDoor1", cmd)) {
		PostEvent(AppEvent::AutoEvent, Vehicle::DoorOpened, 4, 0);
		PostEvent(AppEvent::AutoEvent, Vehicle::DoorOpened, 1, 0);
		return true;
	}
	OPER_DOOR(DoorOpened, "openDoor2", door.lh_rear,  2, true);
	OPER_DOOR(DoorOpened, "openDoor3", door.rh_rear,  3, true);
	OPER_DOOR(DoorOpened, "openCtrl", door.rh_rear, 4, true);
	OPER_DOOR(DoorOpened, "openHood",  door.hood,     5, true);
	OPER_DOOR(DoorOpened, "openLugDoor", door.luggage_door,6, true);
	OPER_DOOR(DoorOpened, "openPowerPlug", door.fuellid,    7, true);

	OPER_DOOR(DoorClosed, "shutDoor0", door.lh_front, 0,false);
	OPER_DOOR(DoorClosed, "shutDoor1", door.rh_front, 1, false);
	OPER_DOOR(DoorClosed, "shutDoor2", door.lh_rear, 2, false);
	OPER_DOOR(DoorClosed, "shutDoor3", door.rh_rear, 3, false);
	OPER_DOOR(DoorClosed, "shutCtrl", door.rh_rear, 4, false);
	OPER_DOOR(DoorClosed, "shutHood", door.hood, 5, false);
	OPER_DOOR(DoorClosed, "shutLugDoor", door.luggage_door, 6, false);
	OPER_DOOR(DoorClosed, "shutPowerPlug", door.fuellid, 7, false);

	OPER_DOOR(WindOpened, "openWind0", window.lh_front, 0, true);
	OPER_DOOR(WindOpened, "openWind1", window.rh_front, 1, true);
	OPER_DOOR(WindOpened, "openWind2", window.lh_rear, 2, true);
	OPER_DOOR(WindOpened, "openWind3", window.rh_rear, 3, true);

	OPER_DOOR(WindClosed, "shutWind0", window.lh_front, 0, false);
	OPER_DOOR(WindClosed, "shutWind1", window.rh_front, 1, false);
	OPER_DOOR(WindClosed, "shutWind2", window.lh_rear, 2, false);
	OPER_DOOR(WindClosed, "shutWind3", window.rh_rear, 3, false);

	OPER_DOOR(Ignite, "Ignite", pedal.ingnition, 0, true);
	OPER_DOOR(UnIgnt, "UnIgnt", pedal.ingnition, 0, false);
	/*if (!strcmp(cmd, "#")) {
		PostEvent(AppEvent::AutoEvent, Vehicle::ShiftLevel, 0, 0);
		return true;
	}
	if (!strcmp(cmd, "P")) {
		PostEvent(AppEvent::AutoEvent, Vehicle::ShiftLevel, 1, 0);
		return true;
	}
	if (!strcmp(cmd, "R")) {
		PostEvent(AppEvent::AutoEvent, Vehicle::ShiftLevel, 2, 0);
		return true;
	}
	if (!strcmp(cmd, "N")) {
		PostEvent(AppEvent::AutoEvent, Vehicle::ShiftLevel, 3, 0);
		return true;
	}
	if (!strcmp(cmd, "D")) {
		PostEvent(AppEvent::AutoEvent, Vehicle::ShiftLevel, 4, 0);
		return true;
	}*/

	//if (!strcmp("openDoor0", cmd)) {
	//	Vehicle::getInstance().getApparatus().vehiState.door.lh_front = true;
	//	PostEvent(AppEvent::AutoEvent, Vehicle::DoorOpened, 0, 0);
	//	LOG_I("AutoEvent Vehicle::DoorOpened 0 Triggered");
	//	return true;
	//}
	if (!strcmp("shutDoor0", cmd)) {
		Vehicle::getInstance().getApparatus().vehiState.door.lh_front = false;
		PostEvent(AppEvent::AutoEvent, Vehicle::DoorClosed, 0, 0);
		LOG_I("AutoEvent Vehicle::DoorOpened Triggered");
		return true;
	}
	if (!strncmp("doorTimeOut", cmd,sizeof("doorTimeOut") - 1))
	{
		int i = atoi(cmd + sizeof("doorTimeOut"));
		Application::getInstance().getConfig().setDoorActivationTimeOut(i);
		LOG_I("doorTimeOut set %d ok", i);
		return true;
	}
	return false;
}

#ifdef ME_DEBUGUI
#ifdef ME_STDIO
using namespace me;
lua_State* g_state;
me::CmdConsole con;
Application* getApp() {
	return &Application::getInstance();
}
void conned() {
	PostEvent(AppEvent::NetStateChanged, 1, 0, 0);
}
void disced() {
	PostEvent(AppEvent::NetStateChanged, 0, 0, 0);
}
void log_traceCallback(enum LOG_LEVELS level, char* message)
{
	if(level >= 4)LOG_I("%d,%s", level, message);
}
#include "./stdinout.cpp"
void* initDebugLib()
{
	using namespace luabridge;
	using namespace me;
	me::Memory::setCheckMemory(vTrue);
	me::Singleton::initAll();
	me::Trace::addTracer(&con);
	g_state = luaL_newstate();         //创建lua运行环境
	if (g_state){
		const lua_Number* n = lua_version(g_state);
		LOG_I("lua %d create ...", *n);
		luaL_openlibs(g_state);
		
		getGlobalNamespace(g_state).beginNamespace("me")
			.addFunction("getApp", &getApp)
			.addFunction("netOk", &conned)
			.addFunction("netFail", &disced)
			.beginClass<Application>("Application")
			.addFunction("connect", &Application::connectServer)
			.addFunction("disconnect", &Application::disconnectServer)
			.endClass();
	}
	
	Log_setTraceCallback(log_traceCallback);
	Log_nameValue lnv[2];
	lnv[0].name = "logname";
	lnv[0].value = "logvalue";
	lnv[1].name = 0;
	Log_initialize(lnv);

	me::Trace::setLevelMask(me::Trace::getLevelMask() & me::Trace::Level::V ? me::Trace::Level::MI : me::Trace::Level::MV);
	return &con;
}
void uninitDebugLib(void* lib)
{
	Log_terminate();
	if (g_state) {
		lua_close(g_state);
	}
	Singleton::destroyAll();
	me::Trace::finalise();
	Memory::dispalyInfo();
	Memory::checkMemory(vTrue);
}
//if (ImGui::SmallButton("reqActive")) { func("reqActive"); } ImGui::SameLine();
//if (ImGui::SmallButton("reqDeact")) { func("reqDeact"); } ImGui::Separator();
char* getCommand(int i)
{
	const char* cmd[] = {
		"ssh","auth", "\0",
		"ntfState","\0",
		"openDoor0", "openDoor1", "openDoor2", "openDoor3", "openCtrl","openHood", "openLugDoor", "openPowerPlug","\0",
		"shutDoor0", "shutDoor1", "shutDoor2", "shutDoor3", "shutCtrl","shutHood", "shutLugDoor", "shutPowerPlug","\0",
		"openWind0","openWind1","openWind2","openWind3","\0",
		"shutWind0","shutWind1","shutWind2","shutWind3","\0",
		"Active","DeActived","Ignite","UnIgnt","\0",
		"reqActive","reqDeact","reqReady","AbnMove","NorMove","\0",
		0
	};
	return (char*)cmd[i];
}
//char* getVCommand(int i) {
//	"name",Vehicle::getInstance().getApparatus().vehiState.door.lh_front
//}
static bool isSsh = false;
void onCommand(char* cmd)
{
	if (me::Tool::isEqual("ssh",cmd)) {
		CreateProcess();
		isSsh = true;
		return;
	}
	if (!isSsh) {
		if (onClientTest(cmd))return;
	}
	else {
		if (!me::Tool::endWith(cmd, ");")) {
			WriteToPipe(cmd);
			return;
		}
	}

	if (me::Tool::isEqual(cmd, "v")) {
		me::Trace::setLevelMask(me::Trace::getLevelMask() & me::Trace::Level::V ? me::Trace::Level::MI : me::Trace::Level::MV);
		return;
	}
	if (me::Tool::isEqual(cmd, "i")) {
		me::Trace::setLevelMask(me::Trace::getLevelMask() & me::Trace::Level::I ? me::Trace::Level::MW : me::Trace::Level::MI);
		return;
	}
	if (me::Tool::isEqual(cmd, "m")) {
		me::Trace::setLevelMask(me::Trace::getLevelMask() & me::Trace::Level::M ? me::Trace::Level::MM : me::Trace::Level::MM);
		return;
	}
	if (me::Tool::isEqual(cmd, "e")) {
		me::Trace::setLevelMask(me::Trace::getLevelMask() & me::Trace::Level::E ? me::Trace::Level::ME : me::Trace::Level::ME);
		return;
	}
	if (me::Tool::isEqual(cmd, "clear")) {
		con.clear();
		return;
	}
	if (me::Tool::isEqual(cmd,"memory")) {
		heap_info*  info =Heap_get_info();
		LOG_I("curr size %d,max size %d", info->current_size, info->max_size);
		return;
	}
	if (me::Tool::isEqual(cmd, "memscan")) {
		Heap_scan(TRACE_PROTOCOL);
		return;
	}
	if (Application::getInstance().onDebugCommand(cmd))return;
	
	if (g_state) {
		int r = luaL_dostring(g_state, cmd);
		if (r)
		{
			LOG_E("Lua return false %s", lua_tostring(g_state, -1));
			lua_pop(g_state, 1); // remove error message
			return;
		}
	}
}
#include "../../console/dxmain.cpp"

#else
void* initDebugLib() { return 0; }
void uninitDebugLib(void* lib) {}



void onCommand(char* cmd)
{
	if (Application::getInstance().onDebugCommand(cmd))return;
}
#include "../../console/dxmain.cpp"
#endif
#else
void* initDebugLib() { return 0; }
void uninitDebugLib(void* lib) {}



void onCommand(char* cmd)
{
	if (onClientTest(cmd))return;
	if (Application::getInstance().onDebugCommand(cmd))return;
}
LoopCallback debugMain(int argc, char* argv[]) { return 0; }

char* getCommand(int i){return 0;}

#endif
