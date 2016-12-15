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

u32 Endian::toU32(u8 v[4])
{
	return BC_PACK_ARRAY4(v);
}

u16 Endian::toU16(u8 v[2])
{
#if BC_ENDIAN== BC_ENDIAN_LITTLE
	u16 ret = v[1] | v[0] << 8;
#else
	u16 ret = v[0] | v[1] << 8;
#endif
	return ret;
}

f32 Endian::toF32(u8 v[4])
{
	union MyUnion
	{
		f32 f;
		u32 u;
	}tmp;
	tmp.u = toU32(v);
	return tmp.f;
}

void Endian::toByte(u8* v, u32 u)
{
#if BC_ENDIAN== BC_ENDIAN_LITTLE
	v[3] = u;
	v[2] = u >> 8;
	v[1] = u >> 16;
	v[0] = u >> 24;
#else
	v[0] = u;
	v[1] = u >> 8;
	v[2] = u >> 16;
	v[3] = u >> 24;
#endif
}

void Endian::toByte(u8* v, u16 u)
{
#if BC_ENDIAN== BC_ENDIAN_LITTLE
	v[1] = u;
	v[0] = u >> 8;
#else
	v[0] = u;
	v[1] = u >> 8;
#endif
}

void Endian::toByte3(u8* v, u32 u)
{
#if BC_ENDIAN== BC_ENDIAN_LITTLE
	v[2] = u >> 8;
	v[1] = u >> 16;
	v[0] = u >> 24;
#else
	v[0] = u;
	v[1] = u >> 8;
	v[2] = u >> 16;
#endif
}
