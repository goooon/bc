#ifndef GUARD_Types_h__
#define GUARD_Types_h__

#include "./Make.h"
#include <typeinfo>
#include <stddef.h>
#include <stdlib.h>

typedef signed int      sint;
typedef unsigned int    uint;

typedef signed char     s8;
typedef unsigned char   u8;
#if BC_COMPILER == BC_COMPILER_GNU
#include <stdint.h>
typedef int16_t         s16;
typedef uint16_t  u16;
typedef int32_t          s32;
typedef uint32_t u32;
typedef u32				 b32;
#else
typedef __int16         s16;
typedef unsigned __int16  u16;
typedef __int32          s32;
typedef unsigned __int32 u32;
typedef u32				 b32;
#endif
#if BC_COMPILER == BC_COMPILER_GNU
typedef long long          s64;
typedef unsigned long long u64;
#else
typedef __int64          s64;
typedef unsigned __int64 u64;
#endif
typedef u64             hash64;

typedef double          f64;
typedef float           f32;

typedef s8              sbyte;
typedef u8              ubyte;
typedef long            slong;
typedef unsigned long   ulong;
typedef f32             freal;
typedef f64             dreal;

typedef wchar_t         wchar;
typedef char            achar;
typedef const achar     cachar;
typedef const wchar     cwchar;
typedef const achar*    castr;
typedef const wchar*    cwstr;
typedef u32             color;

#if BC_COMPILER == BC_COMPILER_GNU
#if BC_TARGET == BC_TARGET_APPLE_IOS
#define bc_size unsigned long
#else
#define bc_size size_t
#endif

#else
#define bc_size size_t

#endif

struct BCMemory
{
	void* operator new(bc_size size);
	//void* operator new(me_size size,void* p);
	void* operator new(bc_size size, int line, const char file[]);
	void* operator new(bc_size size, int line, const char* file, const char* info);
	//void* operator delete(me_size size,void* p);
	void  operator delete(void* p);
	void  operator delete(void* p, int line, const char *func);
	void  operator delete(void* p, int line, const char *func, const char* ext) {};
	//void  operator delete(void* p,const char* fmt,...){}
	void* operator new[]( bc_size size);
	void  operator delete[](void* p);
};
#endif // GUARD_Types_h__
