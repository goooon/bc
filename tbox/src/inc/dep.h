#ifndef MQTT_GUARD_dep_h__
#define MQTT_GUARD_dep_h__

#include "../../../fundation/src/inc/fundation.h"

void* initDebugLib();
void uninitDebugLib(void* lib);
typedef void (*CmdCallback)(char* cmd);
typedef void (*LoopCallback)(CmdCallback onCommand);
#ifdef ME_DEBUGUI
#define ME_STDIO
#define ME_HASLUA
#define WIN
#define _WIN32
#include "../../../../../../goooon/svn/Project/meLib/public/UILib.h"
#include "../../../../../../goooon/svn/Project/external/lua-5.2.3/src/lua.hpp"
//http://vinniefalco.com/LuaBridge/Manual.html
#include "../../../../../../goooon/svn/Project/external/LuaBridge/LuaBridge.h"
#pragma commit(lib,"meLib.lib")
#define bc_new new
#define bc_del delete
#define bc_alloc malloc
#define bc_free  free
LoopCallback debugMain(int argc, char* argv[]);
void onCommand(char* cmd);
#else
#include <stdio.h>
#define LOG_P(fmt,...) do {printf(fmt, ##__VA_ARGS__);}while(0);
#define LOG_V(fmt,...) do {printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define LOG_I(fmt,...) do {printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define LOG_W(fmt,...) do {printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define LOG_E(fmt,...) do {printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define LOG_F(fmt,...) do {printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define DebugCode(c)
struct Memory
{

};
#define bc_new new
#define bc_del delete
#define bc_alloc malloc
#define bc_free  free
LoopCallback debugMain(int argc, char* argv[]);;
void onCommand(char* cmd);
#endif
#endif // GUARD_dep_h__
