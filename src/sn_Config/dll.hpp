#ifndef SN_CONFIG_DLL_H
#define SN_CONFIG_DLL_H

#include "compiler.hpp"

namespace sn_Config {
    namespace dll {

#if defined(SN_CONFIG_COMPILER_MSVC)

#   define SN_CONFIG_DLL_IMPORT __declspec(dllimport)
#   define SN_CONFIG_DLL_EXPORT __declspec(dllexport)
#   define SN_CONFIG_DLL_HIDDEN 


#else

#   define SN_CONFIG_DLL_IMPORT __attribute__((visibility("default")))
#   define SN_CONFIG_DLL_EXPORT __attribute__((visibility("default")))
#   define SN_CONFIG_DLL_HIDDEN __attribute__((visibility("hidden")))



#endif

    }
}


#endif