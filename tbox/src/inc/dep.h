#ifndef MQTT_GUARD_dep_h__
#define MQTT_GUARD_dep_h__

#include "../../../fundation/src/inc/fundation.h"

void* initDebugLib();
void uninitDebugLib(void* lib);
typedef void (*OnCommand)(char* cmd);
typedef void (*LoopBack)(OnCommand onCommand);
#ifdef ME_DEBUGUI
#define ME_STDIO
#define WIN
#define _WIN32
#include "../../../../../../goooon/svn/Project/meLib/public/UILib.h"
#pragma commit(lib,"meLib.lib")
#define bc_new new
#define bc_del delete
#define bc_alloc alloc
#define bc_free  free
LoopBack debugMain(int argc, char* argv[]);
#else
#include <stdio.h>
#define LOG_P(fmt,...) do {printf(fmt, ##__VA_ARGS__);}while(0);
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
#define bc_alloc alloc
#define bc_free  free
LoopBack debugMain(int argc, char* argv[]);;
#endif
#endif // GUARD_dep_h__
