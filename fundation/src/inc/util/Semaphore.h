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
	//��ǰ0����Դ���������1��ͬʱ����,maxCount = 0 ��ΪOPEN
	Semaphore(char* name, int curr = 0, int maxCount = 1);
	ThreadEvent::WaitResult waitP(unsigned int millSecond);
	bool finishV(int cnt = 1);
	~Semaphore();
private:
	ID sid;
};
#endif // GUARD_Semaphore_h__
