#ifndef SN_CONFIG_COMPILER_H
#define SN_CONFIG_COMPILER_H

namespace sn_Config {
    namespace compiler {
        struct msvc {};
        struct gnucc {};
        struct clang {};
        struct other_compiler {};
    }
}

#if defined(_MSC_VER)

typedef sn_Config::compiler::msvc sn_compiler_type;
#define SN_CONFIG_COMPILER_MSVC
#define SN_CONFIG_COMPILER_REPL "MSVC"

#elif defined(__GNUC__) || defined(__GNUG__)

typedef sn_Config::compiler::gnucc sn_compiler_type;
#define SN_CONFIG_COMPILER_GCC
#define SN_CONFIG_COMPILER_REPL "gcc"

#elif defined(__clang__)

typedef sn_Config::compiler::clang sn_compiler_type;
#define SN_CONFIG_COMPILER_CLANG
#define SN_CONFIG_COMPILER_REPL "clang"

#else

typedef sn_Config::compiler::other_compiler sn_compiler_type;
#define SN_CONFIG_COMPILER_UNKNOWN
#define SN_CONFIG_COMPILER_REPL "unknown"

#endif

#endif