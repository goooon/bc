#include "../../inc/util/SharedMemory.h"
#include "../../inc/dep.h"


bool SharedMemory::create(char* name, int size /*= 0*/)
{
#if BC_TARGET == BC_TARGET_LINUX
	key_t msgkey;
	if ((msgkey = ftok(name, proj_id)) == -1) {
		perror("ftok error!\n");
		return false;
	}
	mem_id = shmget(0x1112, size, IPC_CREAT | IPC_EXCL | 0600);
	if (mem_id == -1) {
		return false;
	}
	//把共享内存区对象映射到调用进程的地址空间  
	p_map = shmat(mem_id, NULL, 0);
#elif BC_TARGET == BC_TARGET_WIN
	//////////////////////////////////////////////////////////////////////////
	if (size != 0) {
		mem_id = CreateFileMappingA(
			INVALID_HANDLE_VALUE,    // use paging file
			NULL,                    // default security
			PAGE_READWRITE,          // read/write access
			0,                       // maximum object size (high-order DWORD)
			size,                    // maximum object size (low-order DWORD)
			name);                   // name of mapping object
	}
	else {
		mem_id = OpenFileMappingA(
			FILE_MAP_ALL_ACCESS,   // read/write access
			FALSE,                 // do not inherit the name
			name);                 // name of mapping object
	}
	if (mem_id == NULL)
	{
		LOG_E("Could not create file mapping object (%d).\n", GetLastError());
		return false;
	}
	p_map = (void*)MapViewOfFile(mem_id,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		size);
	if (p_map == NULL)
	{
		LOG_E("Could not map view of file (%d).\n", GetLastError());
		CloseHandle(mem_id);
		mem_id = INVALID_HANDLE_VALUE;
		return false;
	}
#endif
	return true;
}

void SharedMemory::destroy()
{
#if BC_TARGET == BC_TARGET_LINUX
#elif BC_TARGET == BC_TARGET_WIN
	if (p_map != 0) {
		UnmapViewOfFile(p_map);
		p_map = 0;
	}
	if (mem_id != INVALID_HANDLE_VALUE) {
		CloseHandle(mem_id);
		mem_id = INVALID_HANDLE_VALUE;
	}
#endif
}

SharedMemory::~SharedMemory()
{
	destroy();
}
