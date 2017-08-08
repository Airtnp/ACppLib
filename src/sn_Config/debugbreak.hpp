#ifndef SN_CONFIG_DEBUGBREAK_H
#define SN_CONFIG_DEBUGBREAK_H

#if !defined(SN_CONFIG_COMPILER_MSVC)
#include <signal.h>
#endif

namespace sn_Config {
    // ref: https://github.com/scottt/debugbreak/blob/master/debugbreak.h
    namespace debugbreak {

#if defined(SN_CONFIG_COMPILER_MSVC)

#define SN_CONFIG_DEBUG_BREAK __debugbreak

#else

#define SN_CONFIG_DEBUG_BREAK sn_Config::debugbreak::debug_break

#ifdef __cplusplus
        extern "C" {
#endif

// AArch64 iOS
#if defined(__aarch64__) && defined(__APPLE__)

#define SN_CONFIG_DEBUG_BREAK_BUILTIN_TRAP

#else

/* gcc optimizers consider code after __builtin_trap() dead,
* making __builtin_trap() unsuitable for breaking into the debugger */
#define SN_CONFIG_DEBUG_BREAK_SIGTRAP

#endif

#if defined(__i386__) || defined(__x86_64__)

#define SN_CONFIG_HAS_TRAP

            __attribute__((gnu_inline, always_inline))
            __inline__ static void trap_instruction(void) {
                __asm__ volatile("int $0x03");
            }

#elif defined(__thumb__)

#define SN_CONFIG_HAS_TRAP

            __attribute__((gnu_inline, always_inline))
            __inline__ static void trap_instruction(void) {
                /* See 'arm-linux-tdep.c' in GDB source.
                * Both instruction sequences below work. */
#if 1
	            /* 'eabi_linux_thumb_le_breakpoint' */
            	__asm__ volatile(".inst 0xde01");
#else
            	/* 'eabi_linux_thumb2_le_breakpoint' */
	            __asm__ volatile(".inst.w 0xf7f0a000");
#endif

                /* Known problem:
                    * After a breakpoint hit, can't stepi, step, or continue in GDB.
                    * 'step' stuck on the same instruction.
                    *
                    * Workaround: a new GDB command,
                    * 'debugbreak-step' is defined in debugbreak-gdb.py
                    * that does:
                    * (gdb) set $instruction_len = 2
                    * (gdb) tbreak *($pc + $instruction_len)
                    * (gdb) jump   *($pc + $instruction_len)
                */

            }

#elif defined(__arm__)

#define SN_CONFIG_HAS_TRAP

            __attribute__((gnu_inline, always_inline))
            __inline__ static void trap_instruction(void) {
                /* See 'arm-linux-tdep.c' in GDB source,
                * 'eabi_linux_arm_le_breakpoint' */
                __asm__ volatile(".inst 0xe7f001f0");
                /* Has same known problem and workaround
                * as Thumb mode */
            }

#elif defined(__aarch64__) && defined(__APPLE__)

#elif defined(__aarch64__)

#define SN_CONFIG_HAS_TRAP

            __attribute__((gnu_inline, always_inline))
            __inline__ static void trap_instruction(void) {
                /* See 'aarch64-tdep.c' in GDB source,
                * 'aarch64_default_breakpoint' */
                __asm__ volatile(".inst 0xd4200000");
            }

#else

#endif

            __attribute__((gnu_inline, always_inline))
            __inline__ static void debug_break(void) {

#if defined(SN_CONFIG_HAS_TRAP)
                trap_instruction();
#elif defined(SN_CONFIG_DEBUG_BREAK_BUILTIN_TRAP)
                /* raises SIGILL on Linux x86{,-64}, to continue in gdb:
                * (gdb) handle SIGILL stop nopass
                * */
                __builtin_trap();
#else

#ifdef _WIN32
                /* SIGTRAP available only on POSIX-compliant operating systems
                * use builtin trap instead */
                __builtin_trap();
#else
        		raise(SIGTRAP);
#endif

            }



#ifdef __cplusplus
        }
#endif

#endif

    }
}



#endif