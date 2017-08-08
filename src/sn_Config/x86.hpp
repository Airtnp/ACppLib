#ifndef SN_CONFIG_X86_H
#define SN_CONFIG_X86_H

#include <cstdint>
#include <array>
#include <string>
#include "os.hpp"

#if defined(SN_CONFIG_COMPILER_GCC) || defined(SN_CONFIG_COMPILER_CLANG)

#include <cpuid.h>

#elif defined(SN_CONFIG_COMPILER_MSVC)

#include <intrin.h>

#else

#error Unknown compiler

#endif

namespace sn_Config {
    // ref: https://github.com/SuperSodaSea/Corecat/blob/08e0d45a5688385458bd45b856f07dfe8bd8228c/include/Cats/Corecat/X86/X86Feature.hpp
    namespace cpufeature {
        static std::array<std::uint32_t, 4> data cpuid(std::uint32_t func, std::uint32_t sub) {
            std::array<std::uint32_t, 4> data;

#if defined(SN_CONFIG_COMPILER_GCC) || defined(SN_CONFIG_COMPILER_CLANG)
            __cpuid_count(func, sub, data[0], data[1], data[2], data[3]);
#elif defined(SN_CONFIG_COMPILER_MSVC)
            __cpuidex(reinterpret_cast<int*>(data.data()), func, sub);
#endif
            return data;
        }

        static bool cpuid_bit(std::uint32_t func, std::uint32_t sub, std::uint32_t reg, std::uint32_t bit) {
            return (cpuid(func, sub)[reg] >> bit) & 1;
        }

        static const std::uint32_t max_basic_cpuid = cpuid(0x00, 0)[0];
        static const std::uint32_t max_extend_cpuid = cpuid(0x80000000L, 0)[0];
        static const std::string vendor = []{
            char str[12];
            auto data = cpuid(0x00, 0);
            int table[] = {1, 3, 2};
            for (int i = 0; i < 3; ++i) {
                reinterpret_cast<std::uint32_t*>(str)[i] = data[table[i]];
            }
            return std::string(str);
        }();
        static const std::string brand = []{
            char str[48];
            for (int i = 0; i < 3; ++i) {
                auto data = cpuid(0x80000002 + i, 0);
                for (int j = 0; j < 4; ++j) {
                    reinterpret_cast<std::uint32_t*>(str)[i * 4 + j] = data[j];
                }
            }
            return std::string(str);
        }();

        static bool enable_MMX     = cpuid_bit(0x01, 0, 3, 23);
        static bool enable_SSE     = cpuid_bit(0x01, 0, 3, 25);
        static bool enable_SSE2    = cpuid_bit(0x01, 0, 3, 26);
        static bool enable_SSE3    = cpuid_bit(0x01, 0, 2,  0);
        static bool enable_SSSE3   = cpuid_bit(0x01, 0, 2,  9);
        static bool enable_SSSE4_1 = cpuid_bit(0x01, 0, 2, 19);
        static bool enable_SSSE4_2 = cpuid_bit(0x01, 0, 2, 20);
        static bool enable_AVX     = cpuid_bit(0x01, 0, 2, 28);
        static bool enable_AVX2    = cpuid_bit(0x07, 0, 1,  5);
        
    }
}





#endif