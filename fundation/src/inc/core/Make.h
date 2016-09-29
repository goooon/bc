#ifndef GUARD_Make_h__
#define GUARD_Make_h__

#include "./Compiler.h"

#ifdef BC_DEBUG
#define _XTRACE
#define TRACE_MEMORY
#endif

#define _XPROFILE
#if defined(_XTRACE)
#define _XTRACE_LOGFILE
#define _XTRACE_OUTCONSOLE
#define _XTRACE_DBGCONSOLE
#define _XTRACE_MSGBOX
#define BC_LOG_A
#define BC_LOG_PRINTF
#define BC_LOG_PRINT
#define BC_LOG_I
#define BC_LOG_W
#define BC_LOG_E
#define BC_LOG_F
#define BC_LOG_OP
#define _XTRACE_R
#define _XTRACE_THROW_EXCEPTION

#define _XTRACE_FUNCTION
#define _XTRACE_FILE
#define _XTRACE_LINE
#endif

#if BC_TARGET == BC_TARGET_WIN

#elif BC_TARGET == BC_TARGET_LINUX

#endif

/// when define returns true it means that our architecture uses big endian
#define BC_HOST_IS_BIG_ENDIAN (bool)(*(unsigned short *)"\0\xff" < 0x100) 
#define BC_SWAP32(i)  ((i & 0x000000ff) << 24 | (i & 0x0000ff00) << 8 | (i & 0x00ff0000) >> 8 | (i & 0xff000000) >> 24)
#define BC_SWAP16(i)  ((i & 0x00ff) << 8 | (i &0xff00) >> 8)   
#define BC_SWAP_INT32_LITTLE_TO_HOST(i) ((BC_HOST_IS_BIG_ENDIAN)? BC_SWAP32(i) : (i) )
#define BC_SWAP_INT16_LITTLE_TO_HOST(i) ((BC_HOST_IS_BIG_ENDIAN)? BC_SWAP16(i) : (i) )
#define BC_SWAP_INT32_BIG_TO_HOST(i)    ((BC_HOST_IS_BIG_ENDIAN)? (i) : BC_SWAP32(i) )
#define BC_SWAP_INT16_BIG_TO_HOST(i)    ((BC_HOST_IS_BIG_ENDIAN)? (i):  BC_SWAP16(i) )

#define NAMESPACE_BEGIN(n) namespace n{
#define NAMESPACE_END      };
#define USING_NAMESPACE(n) using namespace n;
#endif // GUARD_Make_h__
