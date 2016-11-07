#include "../../inc/core/Types.h"
#include "../../inc/util/Heap.h"
#include <stdlib.h>
#include <stdarg.h>
void* BCMemory::operator new(bc_size size)
{
	return malloc(size);
}

void* BCMemory::operator new(bc_size size, int line, const char file[])
{
	return malloc(size);
}

void* BCMemory::operator new(bc_size size, int line, const char* file, const char* info)
{
	return malloc(size);
}

void BCMemory::operator delete(void* p, int line, const char *func)
{
	free(p);
}

void BCMemory::operator delete(void* p)
{
	free(p);
}

void* BCMemory::operator new[](bc_size size)
{
	return malloc(size);
}

void BCMemory::operator delete[](void* p)
{
	return free(p);
}
