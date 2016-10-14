#ifndef GUARD_Semaphore_h__
#define GUARD_Semaphore_h__
#include "../../inc/dep.h"
#include "../core/Types.h"
#include "./Thread.h"
class Semaphore
{
#if BC_TARGET == BC_TARGET_WIN
	typedef HANDLE ID;
#else
	typedef int ID;
#endif
public:
	//当前0个资源，最大允许1个同时访问,maxCount = 0 则为OPEN
	Semaphore(char* name, int curr = 0, int maxCount = 1);
	ThreadEvent::WaitResult waitP(unsigned int millSecond);
	bool finishV(int cnt = 1);
	~Semaphore();
private:
	ID sid;
};
#endif // GUARD_Semaphore_h__
