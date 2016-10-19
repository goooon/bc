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

#define BC_ENDIAN_LITTLE    1
#define BC_ENDIAN_BIG       2

#if defined(__sgi)      ||  defined (__sparc)        || \
    defined (__sparc__) ||  defined (__PPC__)        || \
    defined (__ppc__)   ||  defined (__BIG_ENDIAN__)
#define BC_ENDIAN BC_ENDIAN_BIG
#else
#define BC_ENDIAN BC_ENDIAN_LITTLE
#endif

#if BC_ENDIAN == BC_ENDIAN_BIG
# define BC_PACK_BYTE4(a,b,c,d) ( (int)(a)<<24 | (int)(b)<<16 | (c)<<8 | (d) )
# define BC_PACK_BYTE2(c, d)   ( (c)<<8 | (d) )
#else
# define BC_PACK_BYTE4(a,b,c,d) ( (int)(d)<<24 | (int)(c)<<16 | (b)<<8 | (a) )
# define BC_PACK_BYTE2(c, d)    ( (d)<<8 | (c) )
#endif

//compiler def
#if (defined(__arm__) && !defined(__thumb__))
#endif


#if (defined(__arm__) && !defined(__thumb__)) || defined(SK_BUILD_FOR_WINCE) || (defined(SK_BUILD_FOR_SYMBIAN) && !defined(__MARM_THUMB__))
/* e.g. the ARM instructions have conditional execution, making tiny branches cheap */
#define BC_CPU_HAS_CONDITIONAL_INSTRUCTION
#endif

#endif // GUARD_Arch_h__
