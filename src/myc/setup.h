#ifndef MC_SETUP_DONE   // setting up some MyC configurations ===============================
#define MC_SETUP_DONE 1



#ifdef _STDLIB_H // if the standard library is included:

typedef size_t Mc_size_t;

#endif


#if defined(__arm__) || defined(__aarch64__) || defined(MC_INITMACRO_FORCE_ARM) // ARM architectures


#define MC_ARCHITECTURE_ARM 1

#endif


#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || \
    defined(__ppc64__) || defined(MC_INITMACRO_FORCE_64BIT) // 64 bit architectures


    #include <stdint.h>

    #define MC_ARCHITECTURE_64BIT 1

    #ifndef _STDLIB_H // if the standard library is not included:

        typedef uint64_t Mc_size_t;

    #endif

    typedef uint64_t Mc_uint_t;

    typedef int64_t Mc_int_t;

    #define MC_FLOAT double



#elif defined(__i386__) || defined(_M_IX86) || defined(__arm__) || defined(MC_INITMACRO_FORCE_32BIT) // 32 bit architectures


    #include <stdint.h>

    #define MC_ARCHITECTURE_32BIT 1

    #ifndef _STDLIB_H // if the standard library is not included:

        typedef uint32_t Mc_size_t;

    #endif

    typedef uint32_t Mc_uint_t;

    typedef int32_t Mc_int_t;

#define MC_FLOAT float


#else // [WARNING] Unknown Architecture, this architecture could be unsupported and, if it doesn't support <stdint.h>, for example, it will lead to undefined behavior

    #ifndef _STDLIB_H

        typedef unsigned int Mc_size_t;

    #endif

    typedef unsigned int Mc_uint_t;

    typedef int Mc_int_t;


#endif // END OF ARCHITECTURE DEFINITION

#endif // END OF SETUP =====================================================
