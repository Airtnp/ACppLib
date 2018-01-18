#ifndef SN_BUILTIN_X86INS_H
#define SN_BUILTIN_X86INS_H

#include "../sn_Config.hpp"

namespace sn_Builtin {
    char compare_and_swap(int* ptr, int i_old, int i_new) {
        unsigned char ret;
#ifndef SN_CONFIG_COMPILER_MSVC
        __asm__ __volatile__ (
            "lock\n"
            "cmpxchg %2 %1\n"
            "sete %0\n"
            : "=q"(ret), "=m"(*ptr)
            : "r"(i_new), "m"(*ptr), "a"(i_old)
            : "memory"
        );
        return ret;
#else
        return InterlockedCompareExchange(reinterpret_cast<volatile long*>(*ptr), i_old, i_new);
        /*
            # WINNT4 https://github.com/ZoloZiak/WinNT4/blob/f5c14e6b42c8f45c20fe88d14c61f9d6e0386b8e/private/ntos/inc/i386.h
            __asm {
                mov eax i_old
                mov ecx ptr
                mov edx i_new
                lock
                cmpxchgl [ecx] edx
                sete ret
            }
            return ret;
        */
#endif
    }

    inline int32_t exchange(int32_t* ptr, int32_t x) {
        __asm__ __volatile__ (
            "lock\n" 
            "xchgl %0, %1\n"
            : "=r" (x), "+m" (*ptr)
            : "r" (x), "m"(*ptr)
            : "memory"
        );
        return x;
    }
}

#endif