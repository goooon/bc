#ifndef GUARD_Make_h__
#define GUARD_Make_h__

#include "./Compiler.h"

#ifdef BE_DEBUG
#define BE_XTRACE
#define TRACE_MEMORY
#endif

#if defined(BE_XTRACE)

#endif

#if BE_TARGET == BE_TARGET_WIN

#elif BE_TARGET == BE_TARGET_LINUX

#endif


#endif

/// when define returns true it means that our architecture uses big endian
#define BE_HOST_IS_BIG_ENDIAN (bool)(*(unsigned short *)"\0\xff" < 0x100) 
#define BE_SWAP32(i)  ((i & 0x000000ff) << 24 | (i & 0x0000ff00) << 8 | (i & 0x00ff0000) >> 8 | (i & 0xff000000) >> 24)
#define BE_SWAP16(i)  ((i & 0x00ff) << 8 | (i &0xff00) >> 8)   
#define BE_SWAP_INT32_LITTLE_TO_HOST(i) ((BE_HOST_IS_BIG_ENDIAN)? BE_SWAP32(i) : (i) )
#define BE_SWAP_INT16_LITTLE_TO_HOST(i) ((BE_HOST_IS_BIG_ENDIAN)? BE_SWAP16(i) : (i) )
#define BE_SWAP_INT32_BIG_TO_HOST(i)    ((BE_HOST_IS_BIG_ENDIAN)? (i) : BE_SWAP32(i) )
#define BE_SWAP_INT16_BIG_TO_HOST(i)    ((BE_HOST_IS_BIG_ENDIAN)? (i):  BE_SWAP16(i) )

#define NAMESPACE_BEGIN(n)
#define NAMESPACE_END
#endif // GUARD_Make_h__
