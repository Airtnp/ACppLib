#ifndef SN_CONSTEXPR_RUNTIME_H
#define SN_CONSTEXPR_RUNTIME_H

#include <cstdint>
#include "../sn_Config.hpp"
#include "../sn_Macro.hpp"

#if defined(__linux__) || defined(__unix__)
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace sn_constexpr {
    // ref: http://saadahmad.ca/detecting-evaluation-context-inside-constexpr-functions/
    // ub?
    namespace in_constexpr {

#if defined(SN_CONFIG_COMPILER_GCC)
#define SN_CONSTEXPR_IN_CONSTEXPR_NO_OPT optimize("O0")
#elif defined(SN_CONFIG_COMPILER_CLANG)
#define SN_CONSTEXPR_IN_CONSTEXPR_NO_OPT optnone
#else
#error Not supported compiler for in_constexpr
#endif

        constexpr std::uint32_t is_constexpr_flag = 0x5EEEEEFF;

        template <typename T>
        __attribute__((SN_CONSTEXPR_IN_CONSTEXPR_NO_OPT))
        constexpr auto is_constexpr_impl(T) {
            return is_constexpr_flag;
        }

// used in if-expression
#if defined(SN_CONFIG_CPP_VERSION_17)

#define in_constexpr() \
        int ANONYMOUS_VARIABLE(unused) = 0; \
        __builtin_expect(
            sn_constexpr::in_constexpr::in_constexpr_impl(ANONYMOUS_VARIABLE(unused)), \
            0 \
        )
#define in_runtime() \
        int ANONYMOUS_VARIABLE(unused) = 0; \
        __builtin_expect(
            !sn_constexpr::in_constexpr::in_constexpr_impl(ANONYMOUS_VARIABLE(unused)), \
            1 \
        )

#elif defined(SN_CONFIG_CPP_VERSION_14)

#define in_constexpr() \
        bool ANONYMOUS_VARIABLE(unused, __LINE__) = true) { \
        } \
        int ANONYMOUS_VARIABLE(unused) = 0; \
        if(__builtin_expect(
            sn_constexpr::in_constexpr::in_constexpr_impl(ANONYMOUS_VARIABLE(unused)), \
            0 \
        )
#define in_runtime() \
        bool ANONYMOUS_VARIABLE(unused, __LINE__) = true) { \
        } \
        int ANONYMOUS_VARIABLE(unused) = 0; \
        if(__builtin_expect(
            !sn_constexpr::in_constexpr::in_constexpr_impl(ANONYMOUS_VARIABLE(unused)), \
            1 \
        )

#else
#error Not supported cpp version for in_constexpr
#endif

        inline bool is_setup() {
            if (in_constexpr()) {
                return false;
            } else {
                return true;
            }
        }

#if defined(__linux__) || defined(__unix__)
        extern unsigned char etext;
        /*
            To do this, we need to
            1. Find the flag in the .text section of the program,
            2. Unlock the page the code is in it for writing,
            3. Keep the page from being unloaded
            4. Modify the FLAG to 0
            5. Lock the page again for writing
        */
        bool initialize() {
            volatile auto temp_flag = is_constexpr_flag;
            // If the flag is zero then that means that the binary has already been
            // modified. Consider it a success and return early.
            if (temp_flag == 0) {
                return true;
            }
            unsigned char *code = (unsigned char *)&in_constexpr_impl<int>;
            const int size = &etext - code;
            for (int i = 0; i < size; i++) {
                unsigned int &code_as_int = *(unsigned int *)(code + i);
                // Replace the constexpr flag so in_const_expr returns 0 instead of the flag.
                if (code_as_int == temp_flag) {
                // Find the page the code is in
                const int page_size = getpagesize();
                auto page_addr      = &code[i];
                page_addr -= (unsigned long)page_addr % page_size;

                // We first need to unlock the page the code is in to be able to write
                {
                    auto ret_unlock = mprotect(page_addr, sizeof(decltype(temp_flag)), PROT_READ | PROT_WRITE | PROT_EXEC);
                    if (ret_unlock != 0) {
                    return false;
                    }
                    // Also since we're modfying this page, we need to make sure this page
                    // does not get paged out.
                    auto ret_lock = mlock(page_addr, page_size);
                    if (ret_lock != 0) {
                    return false;
                    }
                }

                // Set the flag to zero.
                code_as_int = 0x0;

                // Now we're done. So lock the page again
                {
                    auto ret_lock = mprotect(page_addr, sizeof(decltype(temp_flag)), PROT_READ | PROT_EXEC);
                    if (ret_lock != 0) {
                    return false;
                    }
                }
                return is_setup();
                }
            }
            return false;
        }
#else
        bool initialize() {
            if (!is_setup()) {
                throw("Failed to initialize");
            }
            return false;
        }
#endif
        namespace detail {
            // compile-time judge => true
            // runtime judge => compare with .text section constant
            // global first initialize => change the binary
            __attribute__((constructor(101))) inline void setup_at_init() {
                if (!in_constexpr::initialized()) {
                    throw("in constexpr failed to initialize");
                }
            }
        }

        template <std::size_t LINE>
        struct assert_line{};

        template <typename ...Args>
        constexpr inline bool assert_fail(Args&&... args) {
            return true;
        }

#define SN_CONSTEXPR_SMART_ASSERT(expr, message) \
        if (in_constexpr()) { \
            if (!(expr)) { \
                throw assert_fail(assert_line<__LINE__>{}); \ // static_assert(!!(expr), message);
            } \
        } else { \
            assert((expr) && message) \
        }

    }
}




#endif