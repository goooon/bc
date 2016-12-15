#ifndef GUARD_CanBus_h__
#define GUARD_CanBus_h__

#include "./dep.h"
#include "../../../fundation/src/inc/fundation.h"
#include "./Operation.h"
class CanBus
{
public:
	static CanBus& getInstance();
public:
	Operation::Result reqActiveDoor(bool active);
	Operation::Result reqEnterReadyToIgnite(bool ready);
	Operation::Result reqOpenWindow(u8 idx,bool open);
public:
	bool getStateBlocked(u8 idx,u8 size,u8* data);
};
#endif // GUARD_CanBus_h__
