#ifndef GUARD_Arch_h__
#define GUARD_Arch_h__

#define BC_TARGET_WIN       1
#define BC_TARGET_LINUX     2
#define BC_TARGET_ANDROID   3

#ifndef BC_TARGET
#if defined (_WIN32)
# define BC_TARGET BC_TARGET_WIN
#elif defined(__APPLE__)
# if __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 30000 || __IPHONE_OS_VERSION_MIN_REQUIRED >= 30000
#   define BC_TARGET BC_TARGET_APPLE_IOS
# else
#	define MBC_TARGET BC_TARGET_APPLE_IOS
# endif
#elif defined(__ANDROID__)
# define BC_TARGET BC_TARGET_ANDROID
#else
# define BC_TARGET BC_TARGET_LINUX
#endif
#endif

#define BC_ARCH_32 1
#define BC_ARCH_64 2

#if defined(__x86_64__)     || defined(_M_X64)      || \
    defined(__powerpc64__)  || defined(__alpha__)   || \
    defined(__ia64__)       || defined(__s390__)    || \
    defined(__s390x__)
# define BC_ARCH BC_ARCH_64
#if defined(WIN32) || defined(WIN64)
static_assert(sizeof(void *) == 8, "void*_should_be_8");
#endif
#else
#define BC_ARCH BC_ARCH_32
#if defined(WIN32) || defined(WIN64)
static_assert(sizeof(void *) == 4, "void*_should_be_4");
#endif
#endif

#define BC_ENDIAN_UNKNOWN   0
#define BC_ENDIAN_LITTLE    1
#define BC_ENDIAN_BIG       2

//https://sourceforge.net/p/predef/wiki/Architectures/
//https://msdn.microsoft.com/en-us/library/b0084kay.aspx
#if defined(__sgi)      ||  defined (__sparc)        || \
    defined (__sparc__) ||  defined (__PPC__)        || \
    defined (__ppc__)   ||  defined (__BIG_ENDIAN__)
#define BC_ENDIAN BC_ENDIAN_BIG
#elif defined(__arm__) || defined(__i386) || defined(__i386__) || defined(__ia64__)  || \
	defined(_M_IX86 )
#define BC_ENDIAN BC_ENDIAN_LITTLE
#else
#define BC_ENDIAN BC_ENDIAN_UNKNOWN
#endif

#if BC_ENDIAN == BC_ENDIAN_BIG
#define  BC_PACK_ARRAY4(b) ( (int)(b[0])<<24 | (int)(b[1])<<16 | (b[2])<<8 | (b[3]) )
# define BC_PACK_ARRAY2(b)   ( (b[0])<<8 | (b[1]) )
# define BC_PACK_BYTE4(b0,b1,b2,b3) ( (int)(b0)<<24 | (int)(b1)<<16 | (b2)<<8 | (b3) )
# define BC_PACK_BYTE2(b0,b1)   ( (b0)<<8 | (b1) )
#elif BC_ENDIAN == BC_ENDIAN_LITTLE
#define  BC_PACK_ARRAY4(b) ( (int)(b[3])<<24 | (int)(b[2])<<16 | (b[1])<<8 | (b[0]) )
# define BC_PACK_ARRAY2(b)    ( (b[1])<<8 | (b[0]) )
# define BC_PACK_BYTE4(b0,b1,b2,b3) ( (int)(b3)<<24 | (int)(b2)<<16 | (b1)<<8 | (b0) )
# define BC_PACK_BYTE2(b0, b1)    ( (b1)<<8 | (b0) )
#else
#define  BC_PACK_ARRAY4(b)  ((bool)(*(unsigned short *)"\0\xff" < 0x100) ? ((int)(b[0])<<24 | (int)(b[1])<<16 | (b[2])<<8 |(b[3] ) : ( (int)(b[3])<<24 | (int)(b[2])<<16 | (b[]1)<<8 | (b[0]) ))
# define BC_PACK_ARRAY2(b)  ((bool)(*(unsigned short *)"\0\xff" < 0x100) ? ((b[0])<<8 | (b[1]) ) : ( (b[1])<<8 | (b[0]) ))
# define BC_PACK_BYTE4(b0,b1,b2,b3) ((bool)(*(unsigned short *)"\0\xff" < 0x100) ? ( (int)(b0)<<24 | (int)(b1)<<16 | (b2)<<8 | (b3) ) : ( (int)(b3)<<24 | (int)(b2)<<16 | (b1)<<8 | (b0) ))
# define BC_PACK_BYTE2(b0, b1)    ((bool)(*(unsigned short *)"\0\xff" < 0x100) ? ( (b0)<<8 | (b1) ) : ( (b1)<<8 | (b0) ))
#endif

//compiler def
#if (defined(__arm__) && !defined(__thumb__))
#endif


#if (defined(__arm__) && !defined(__thumb__)) || defined(SK_BUILD_FOR_WINCE) || (defined(SK_BUILD_FOR_SYMBIAN) && !defined(__MARM_THUMB__))
/* e.g. the ARM instructions have conditional execution, making tiny branches cheap */
#define BC_CPU_HAS_CONDITIONAL_INSTRUCTION
#endif

#endif // GUARD_Arch_h__
