#include "../inc/dep.h"
#include "../inc/Application.h"
#include "../inc/Event.h"
#include <errno.h>
unsigned int last_error(void)
{
#if defined(WIN32) || defined(WIN64)
	return (unsigned int)GetLastError();
#else
	return errno;
#endif
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
	PostEvent(AppEvent::NetConnected, 0, 0, 0);
}
void disced() {
	PostEvent(AppEvent::NetDisconnected, 0, 0, 0);
}
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
	return &con;
}
void uninitDebugLib(void* lib)
{
	if (g_state) {
		lua_close(g_state);
	}
	Singleton::destroyAll();
	me::Trace::finalise();
	Memory::dispalyInfo();
	Memory::checkMemory(vTrue);
}
void onCommand(char* cmd)
{
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
	if (Application::getInstance().onDebugCommand(cmd))return;
}
LoopCallback debugMain(int argc, char* argv[]) { return 0; }
#endif
