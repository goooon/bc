#ifndef MQTT_GUARD_dep_h__
#define MQTT_GUARD_dep_h__

#include "../../../fundation/src/inc/fundation.h"
#include "../../../fundation/src/inc/bcp.h"
#include "../../../fundation/src/inc/vicp/bcp_vicp.h"

#if defined(WIN32) || defined(WIN64)
#define OVERRIDE override
#define NULLPTR	nullptr
#else
#define OVERRIDE
#define NULLPTR NULL
#endif

unsigned int last_error(void);

void* initDebugLib();
void  uninitDebugLib(void* lib);
typedef char* (*GetCommand)(int idx);
typedef void (*CmdCallback)(char* cmd);
typedef void (*LoopCallback)(GetCommand getCommand, CmdCallback onCommand, GetCommand getVCommand, CmdCallback onVCommand);

#ifdef ME_DEBUGUI
#include "./incdebug.h"
#define bc_new new(__LINE__,__FILE__)
#define bc_del delete
#define bc_alloc malloc
#define bc_free  free
LoopCallback debugMain(int argc, char* argv[]);
char* getCommand(int i);
void onCommand(char* cmd);
#else
#include <stdio.h>
#ifdef LOG_A
#undef LOG_A
#endif
#ifdef LOG_P
#undef LOG_P
#endif
#ifdef LOG_V
#undef LOG_V
#endif
#ifdef LOG_I
#undef LOG_I
#endif
#ifdef LOG_W
#undef LOG_W
#endif
#ifdef LOG_E
#undef LOG_E
#endif
#ifdef LOG_F
#undef LOG_F
#endif
#define LOG_A(expr,...) if(!expr)printf(__VA_ARGS__);
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
LoopCallback debugMain(int argc, char* argv[]);
void onCommand(char* cmd);
char* getCommand(int i);
#endif
#endif // GUARD_dep_h__
