#ifndef MQTT_GUARD_dep_h__
#define MQTT_GUARD_dep_h__

#include "../../../fundation/src/inc/fundation.h"

void* initMeLib();
void uninitMeLib(void* lib);
#if 0
#define ME_STDIO
#define WIN
#define _WIN32
#include "../../../../../../goooon/svn/Project/meLib/public/UILib.h"
#pragma commit(lib,"meLib.lib")

#else
#include <stdio.h>
#define LOG_I(fmt,...) do {printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define LOG_W(fmt,...) do {printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define LOG_E(fmt,...) do {printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define LOG_F(fmt,...) do {printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define DebugCode(c)
#endif
#endif // GUARD_dep_h__
