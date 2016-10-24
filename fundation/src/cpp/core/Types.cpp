#include "../../inc/core/Types.h"
#include "../../inc/util/Heap.h"
#include <stdlib.h>
#include <stdarg.h>
void* BCMemory::operator new(bc_size size)
{
	return mymalloc(__FILE__, __LINE__, size);
}

void* BCMemory::operator new(bc_size size, int line, const char file[])
{
	return mymalloc( (char*)file, line,size);
}

void* BCMemory::operator new(bc_size size, int line, const char* file, const char* info)
{
	return mymalloc( (char*)file, line,size);
}

void BCMemory::operator delete(void* p, int line, const char *func)
{
	myfree((char*)func, line, p);
}

void BCMemory::operator delete(void* p)
{
	myfree(__FILE__,__LINE__,p);
}

void* BCMemory::operator new[](bc_size size)
{
	return mymalloc(__FILE__, __LINE__, size);
}

void BCMemory::operator delete[](void* p)
{
	return myfree(__FILE__, __LINE__, p);
}
