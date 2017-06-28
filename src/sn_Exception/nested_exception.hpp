#ifndef SN_EXCEPTION_NESTED_EXCEPTION_H
#define SN_EXCEPTION_NESTED_EXCEPTION_H

#include "../sn_CommonHeader.h"

// incomplete
namespace sn_Exception {

#ifdef SN_ENABLE_CPP_17_EXPERIMENTAL

    template <typename T, typename D>
    struct with_nested : public D, public std::nested_exception {
        explicit with_nested(T&& v)
            : D{std::forward<T>(v)}, std::nested_exception{} {}
        // rethrow exception_ptr
    }

    template <typename T>
    [[noreturn]] void throw_with_nested_impl(T&& v, std::true_type) {
        using D = typename std::decay_t<T>;
        using G = with_nested<T, D>;
        throw G{std::forward<T>(value)};
    }

    template <typename T>
    [[noreturn]] void throw_with_nested_impl(T&& v, std::false_type) {
        using D = typename std::decay_t<T>;
        using G = with_nested<T, D>;
        throw G{std::forward<T>(value)};
    }

#endif
}


#endif