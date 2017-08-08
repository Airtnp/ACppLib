#ifndef SN_CONFIG_CPPV_H
#define SN_CONFIG_CPPV_H

namespace sn_Config {
    namespace cpp_version {
        struct cpp17 {};
        struct cpp14 {};
        struct cpp11 {};
        struct cpp03 {};
        struct cpp98 {};
        struct other_cpp {};
    }
}

#if __cplusplus > 20170301L
    
typedef sn_Config::cpp_version::cpp17 sn_cpp_version_type;
#define SN_CPP_VERSION_17
#define SN_CPP_VERSION_REPL 17

#elif __cplusplus > 20140301L

typedef sn_Config::cpp_version::cpp14 sn_cpp_version_type;
#define SN_CPP_VERSION_14
#define SN_CPP_VERSION_REPL 14

#elif __cplusplus > 20110301L

typedef sn_Config::cpp_version::cpp11 sn_cpp_version_type;
#define SN_CPP_VERSION_11
#define SN_CPP_VERSION_REPL 11

#elif __cplusplus > 20030301L

typedef sn_Config::cpp_version::cpp03 sn_cpp_version_type;
#define SN_CPP_VERSION_03
#define SN_CPP_VERSION_REPL 03

#elif __cplusplus > 19980301L

typedef sn_Config::cpp_version::cpp98 sn_cpp_version_type;
#define SN_CPP_VERSION_98
#define SN_CPP_VERSION_REPL 98

#else

typedef sn_Config::cpp_version::other_cpp sn_cpp_version_type;
#define SN_CPP_VERSION_UNKNOWN
#define SN_CPP_VERSION_REPL "unknown"

#endif




#endif