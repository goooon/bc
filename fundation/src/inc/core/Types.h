#ifndef GUARD_Types_h__
#define GUARD_Types_h__

#include "./Make.h"
#include <typeinfo>
#include <stddef.h>
#include <stdlib.h>

NAMESPACE_BEGIN(bc)

	typedef signed int      sint;
	typedef unsigned int    uint;

	typedef signed char     s8;
	typedef unsigned char   u8;
	typedef __int16         s16;
	typedef unsigned __int16  u16;
	typedef __int32          s32;
	typedef unsigned __int32 u32;

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

NAMESPACE_END
#endif // GUARD_Types_h__
