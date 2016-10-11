#ifndef MQTT_GUARD_dep_h__
#define MQTT_GUARD_dep_h__

#include "../../../fundation/src/inc/fundation.h"

void* initDebugLib();
void uninitDebugLib(void* lib);
typedef void (*CmdCallback)(char* cmd);
typedef void (*LoopCallback)(CmdCallback onCommand);
#ifdef ME_DEBUGUI
#include "./incdebug.h"
#define bc_new new
#define bc_del delete
#define bc_alloc malloc
#define bc_free  free
LoopCallback debugMain(int argc, char* argv[]);
void onCommand(char* cmd);
#else
#include <stdio.h>
#define LOG_A(expr,...) if(!expr)printf(##__VA_ARGS__);
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
