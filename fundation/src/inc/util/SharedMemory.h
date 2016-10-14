#ifndef GUARD_SharedMemory_h__
#define GUARD_SharedMemory_h__
#include "./Semaphore.h"
#include "../core/Types.h"

class SharedMemory
{
#if BC_TARGET == BC_TARGET_WIN
	typedef HANDLE ID;
#else
	typedef int ID;
#endif
public:
	//size = 0 means open
	bool create(char* name,int size = 0);
	void destroy();
	~SharedMemory();
private:
	ID mem_id;
	void* p_map;
	Semaphore sem;
};
#endif // GUARD_SharedMemory_h__
