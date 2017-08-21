#ifndef SN_CONFIG_MISC_H
#define SN_CONFIG_MISC_H

#include "compiler.hpp"

namespace sn_Config {
    namespace misc {

#if defined(SN_CONFIG_COMPILER_MSVC)

#define SN_CONFIG_ALIGN_BEGIN(size) __declspec(align(size)) // use #pragma push/pop
#define SN_CONFIG_ALIGN_END(size)

#else

#define SN_CONFIG_ALIGN_BEGIN(size)  // use #pragma push/pop
#define SN_CONFIG_ALIGN_END(size) __attribute__((aligned(size)))

#endif


    }
}

#endif