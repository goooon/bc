#ifndef GUARD_Compiler_h__
#define GUARD_Compiler_h__

#include "./Arch.h"

#if  defined(DEBUG) ||  defined (_DEBUG)
#define BC_DEBUG
#else
#define BC_RELEASE
#endif
//////////////////////////////////////////////////////////////////////////

#if BC_TARGET == BC_TARGET_WIN

# if defined(__MINGW32__) || \
    defined(__CYGWIN__)  || \
    (defined (_MSC_VER) && _MSC_VER < 1300)
#  define BC_INLINE inline
# else
#  define BC_INLINE __forceinline
# endif
#else
#  define BC_INLINE    inline
#endif

#define BC_COMPILER_MSVC    0
#define BC_COMPILER_GNU     1

#if defined(_MSC_VER)
# define BC_COMPILER BC_COMPILER_MSVC
#pragma  warning(once:4458) //show only one warning
#elif defined(__GNUC__)
# define BC_COMPILER BC_COMPILER_GNU
#else
# error unknown compiler
#endif

#if BC_COMPILER == BC_COMPILER_MSVC
// Function type macros.
#define BC_VARARGS            __cdecl                    /* Functions with variable arguments */
#define BC_CDECL              __cdecl                    /* Standard C function */
#define BC_STDCALL            __stdcall                /* Standard calling convention */
#define BC_FORCEINLINE        __forceinline            /* Force code to be inline */
#define BC_FORCENOINLINE      __declspec(noinline)    /* Force code to NOT be inline */

// Hints compiler that expression is true; generally restricted to comparisons against constants
#define ASSUME(expr)...       __assume(expr)

// Alignment.
//#define MS_ALIGN(n)
#ifdef CLR
#define MS_ALIGN(n)
#define GCC_PACK(n)
#else
#define MS_ALIGN(n) __declspec(align(n))
#define GCC_PACK(n)
#endif

#ifndef BC_LIB
#ifdef BC_DLL
#define BC_API __declspec(dllexport)
#else
#define BC_API __declspec(dllimport)
#endif
#else
#define BC_API  
#endif

// disable this now as it is annoying for generic platform implementations
#pragma warning(disable : 4100) // unreferenced formal parameter
#endif

#if BC_COMPILER == BC_COMPILER_MSVC
#define DECL_MSC_ALIGNED_CLASS(c,n) __declspec(align(n)) c
#define DECL_MSC_ALIGN(n) __declspec(align(n))
#define DECL_GNU_ALIGNED_CLASS(c,n) 
#define DECL_GNU_ALIGN(n) 
#elif BC_COMPILER BC_COMPILER_GNU
#define DECL_MSC_ALIGNED_CLASS(c,n) 
#define DECL_MSC_ALIGN(n) 
#define DECL_GNU_ALIGNED_CLASS(c,n) c __attribute__ ((packed,aligned (n)))	
#define DECL_GNU_ALIGN(n) __attribute__ ((packed,aligned (n)))
#else
#define DECL_MSC_ALIGNED_CLASS(c,n) 
#define DECL_MSC_ALIGN(n) 
#define DECL_GNU_ALIGNED_CLASS(c,n) 
#define DECL_GNU_ALIGN(n)
#endif

#endif // GUARD_Compiler_h__
