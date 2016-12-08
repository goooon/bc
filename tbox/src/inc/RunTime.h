#ifndef GUARD_RunTime_h__
#define GUARD_RunTime_h__

#include "./dep.h"
#include "./Operation.h"
#include "./Apparatus.h"
#include "../tasks/Element.h"

class RunTime
{
public:
	static RunTime& getInstance() {
		static RunTime rt;
		return rt;
	}
	RunTime():stateItemCount(1), debugCollide(1){}
public:
	u8 getStateItems(u8** ps) {
		if (ps)*ps = stateItems;
		return stateItemCount;
	}
public:
	u32  debugCollide;
	bool debugState;
	u8 stateItemCount;
	u8 stateItems[256];
};
#endif // GUARD_RunTime_h__
