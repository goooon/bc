#ifndef GUARD_RunTime_h__
#define GUARD_RunTime_h__

#include "./dep.h"
#include "./Operation.h"
#include "./Apparatus.h"
#include "../tasks/Element.h"

struct DTCReqDesc
{
	u8 dtcType;
	u8 ecuNumb;
	u8 ecuIndex[1];
}DECL_GNU_PACKED;

class RunTime
{
public:
	static RunTime& getInstance() {
		static RunTime rt;
		return rt;
	}
	RunTime();
public:
	u8 getStateItems(u8** ps) {
		if (ps)*ps = stateItems;
		return stateItemCount;
	}
	void control(u8 id, u8 arg);
	bool getControlResult(u8 id, u8& result);
	void reqDiagnose(DTCReqDesc* req);
public:
	u32  debugCollide;
	int  collideLevel[10];

	bool debugState;
	u8 stateItemCount;
	u8 stateItems[256];
	u8 controlarg[256];

	u8 diagEcuCount;
	u8 diagEcuValid[256];
	u8 diagEcuError[256];
	u8 diagEcuIndex[256];
	int diagDTCCount[256];
	u16 diagDTCCode[256][256 * 256];
};
#endif // GUARD_RunTime_h__
