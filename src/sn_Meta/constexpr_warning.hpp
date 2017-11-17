#ifndef SN_META_CONSTEXPR_WARNING_H
#define SN_META_CONSTEXPR_WARNING_H

#include "../sn_Macro.hpp"

namespace sn_Meta {
    // ref: https://stackoverflow.com/questions/8936063/does-there-exist-a-static-warning/9018194#9018194?utm_source=qq&utm_medium=social
    namespace constexpr_warning {

#if defined(__GNUC__)
#define SN_CONSTEXPR_DEPRECATE(foo, msg) foo __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
#define SN_CONSTEXPR_DEPRECATE(foo, msg) __declspec(deprecated(msg)) foo
#else
#error This compiler is not supported
#endif

        namespace detail {
            struct true_type {};
            struct false_type {};
            template <int test> struct converter : public true_type {};
            template <> struct converter<0> : public false_type {};
        }

#define SN_CONSTEXPR_STATIC_WARNING(cond, msg) \
struct MACRO_RAW_CONCAT(static_warning,__LINE__) { \
    SN_CONSTEXPR_DEPRECATE(void _(::detail::false_type const& ),msg) {}; \
    void _(::detail::true_type const& ) {}; \
    MACRO_RAW_CONCAT_CAT(static_warning,__LINE__)() {_(::detail::converter<(cond)>());} \
}

// Note: using STATIC_WARNING_TEMPLATE changes the meaning of a program in a small way.
// It introduces a member/variable declaration.  This means at least one byte of space
// in each structure/class instantiation.  STATIC_WARNING should be preferred in any 
// non-template situation.
//  'token' must be a program-wide unique identifier.
#define SN_CONSTEXPR_STATIC_WARNING_TEMPLATE(token, cond, msg) \
    SN_CONSTEXPR_STATIC_WARNING(cond, msg) MACRO_RAW_CONCAT(MACRO_RAW_CONCAT(_localvar_, token),__LINE__)
    }
}

#endif