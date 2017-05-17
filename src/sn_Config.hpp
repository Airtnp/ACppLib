#ifndef SN_CONFIG_H
#define SN_CONFIG_H

#include "sn_CommonHeader.h"

struct windows32 {};
struct windows64 {};
struct macosx {};
struct linux {};
struct other_os {};

#if defined(_WIN64)
    typedef windows64 platform_type;
#elif defined(_WIN32)
    typedef windows32 platform_type;
#elif defined(__APPLE__) && defined(__MACH__)
    typedef macosx platform_type;
#elif defined(__linux__)
    typedef linux platform_type;
#else
    typedef other_os platform_type;
#endif


struct vcpp {};
struct gnucc {};
struct clang {};
struct other_compiler {};

#if defined(_MSC_VER)
    typedef vcpp compiler_type;
#elif defined(__GNUC__) || defined(__GNUG__)
    typedef gnucc compiler_type;
#elif defined(__clang__)
    typedef clang compiler_type;
#else
    typedef other_compiler compiler_type;
#endif

struct cpp17 {};
struct cpp14 {};
struct cpp11 {};
struct cpp03 {};
struct cpp98 {};
struct other_cpp {};

#if __cplusplus > 20170301L
    typedef cpp17 cpp_type;
#else

#endif

struct debugT {};
struct releaseT {};

#if defined(_DEBUG)
    typedef debugT compiler_optimzation_option;
#else
    typedef releaseT compiler_optimzation_option;
#endif

#endif