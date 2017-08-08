#ifndef SN_CONFIG_OS_H
#define SN_CONFIG_OS_H

namespace sn_Config {
    namespace os {
        struct windows32 {};
        struct windows64 {};
        struct macosx {};
        struct linux {};
        struct other_os {};
    }
}

#if defined(_WIN64)

typedef sn_Config::os::windows64 sn_platform_type;
#define SN_CONFIG_OS_WIN64
#define SN_CONFIG_OS_REPL "windows64"

#elif defined(_WIN32)

typedef sn_Config::os::windows32 sn_platform_type;
#define SN_CONFIG_OS_WIN32
#define SN_CONFIG_OS_REPL "windows32"

#elif defined(__APPLE__) && defined(__MACH__)

typedef sn_Config::os::macosx sn_platform_type;
#define SN_CONFIG_OS_MACOSX
#define SN_CONFIG_OS_REPL "Mac OS X"

#elif defined(__linux__)

typedef sn_Config::os::linux sn_platform_type;
#define SN_CONFIG_OS_LINUX
#define SN_CONFIG_OS_REPL "Linux"

#else

typedef sn_Config::os::other_os platform_type;
#define SN_CONFIG_OS_UNKNOWN
#define SN_CONFIG_OS_REPL "unknown"

#endif

#endif