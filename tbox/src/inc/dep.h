#ifndef MQTT_GUARD_dep_h__
#define MQTT_GUARD_dep_h__

#include "../../../fundation/src/inc/fundation.h"

void* initMeLib();
void uninitMeLib(void* lib);
#if 1
#define ME_STDIO
#define WIN
#define _WIN32
#include "../../../../../../goooon/svn/Project/meLib/public/UILib.h"
#pragma commit(lib,"meLib.lib")

#else
#include <stdio.h>
#define LOG_I printf
#define LOG_W printf
#define LOG_E printf
#define LOG_F printf
#define DebugCode(c)
#endif
#endif // GUARD_dep_h__
