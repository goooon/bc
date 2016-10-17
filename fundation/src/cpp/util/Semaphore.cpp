#include "../../inc/util/Semaphore.h"

#if BC_TARGET == BC_TARGET_LINUX
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#endif

Semaphore::Semaphore(char* name, int curr /*= 0*/, int maxCount /*= 1*/)
{
#if BC_TARGET == BC_TARGET_LINUX
	key_t msgkey;
	int proj_id = 1;

	if ((msgkey = ftok(name, proj_id)) == -1) {
		perror("ftok error!\n");
		return;
	}

	if ((sid = semget(msgkey, 0, 0666)) == -1) {
		perror("open semget call failed.\n");
		return;
	}
	/*设置信号量的初始值，就是资源个数*/
	union semun {
		int val;
		struct semid_ds *buf;
		ushort *array;
	}sem_u;
	sem_u.val = maxCount; /* TODO:  */
	semctl(sid, 0, SETVAL, sem_u);
#elif BC_TARGET == BC_TARGET_WIN
	if (maxCount != 0) {
		sid = CreateSemaphoreA(NULL, curr, maxCount, name);
	}
	else {
		sid = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, true, name);
	}
#endif
}

ThreadEvent::WaitResult Semaphore::waitP(unsigned int millSecond)
{//P
#if BC_TARGET == BC_TARGET_LINUX
	struct sembuf sb;
	sb.sem_num = 0;
	sb.sem_op = -1; /* TODO: -cnt; */
	sb.sem_flg = 0;
	semop(sid, &sb, 1);
#elif BC_TARGET == BC_TARGET_WIN
	DWORD ret = WaitForSingleObject(sid, millSecond);//等待信号量>0  
	if (ret == WAIT_TIMEOUT)return ThreadEvent::TimeOut;
	if (ret == WAIT_OBJECT_0)return ThreadEvent::EventOk;
	LOG_E("WaitForSingleObject failed %d", GetLastError());
#endif
	return ThreadEvent::Errors;
}

bool Semaphore::finishV(int cnt /*= 1*/)
{//V
	bool ret;
#if BC_TARGET == BC_TARGET_LINUX
	struct sembuf sb;
	sb.sem_num = 0;
	sb.sem_op = cnt;
	sb.sem_flg = 0;
	semop(sid, &sb, 1);
	ret = true;
#elif BC_TARGET == BC_TARGET_WIN
	ret = ReleaseSemaphore(sid, cnt, NULL) ? true : false;//信号量++ 
	if (!ret) {
		LOG_E("ReleaseSemaphore failed %d", GetLastError());
	}
#endif
	return ret;
}

Semaphore::~Semaphore()
{
	#if BC_TARGET == BC_TARGET_LINUX
	
	#elif BC_TARGET == BC_TARGET_WIN
	if (sid != INVALID_HANDLE_VALUE) {
		CloseHandle(sid);
	}
	#endif
}