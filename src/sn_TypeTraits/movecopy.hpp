#ifndef SN_TYPETRAITS_MOVECOPY_H
#define SN_TYPETRAITS_MOVECOPY_H

#include<type_traits>

namespace sn_TypeTraits {
    template <typename T>
    using prefer_copy_over_move_constructible = 
        std::conditional_t<
            (!std::is_nothrow_move_constructible<T>::value && std::is_nothrow_copy_constructible<T>::value) ||
            (!std::is_move_constructible<T>::value && std::is_copy_constructible<T>::value),
            std::true_type,
            std::false_type
        >;
    template <typename T>
    using prefer_move_over_copy_constructible = 
        std::conditional_t<
            std::is_nothrow_move_constructible<T>::value ||
            (!std::is_nothrow_copy_constructible<T>::value && std::is_move_constructible<T>::value),
            std::true_type,
            std::false_type
        >;
    // memmove | memcpy
    template <typename T>
    using prefer_trivially_constructible = std::is_trivially_constructible;
    template <typename T>
    using is_copy_or_move_constructble = 
        std::conditional_t<
            std::is_trivially_constructible<T>::value ||
            std::is_copy_constructible<T>::value ||
            std::is_move_constructible<T>::value
        >;
    template <typename T>
    using is_move_or_copy_constructble = is_copy_or_move_constructble;

    template <typename T>
    using prefer_move_over_copy_assignable = 
        std::conditional_t<
            std::is_nothrow_move_assignable<T>::value ||
            (!std::is_nothrow_copy_assignable<T>::value && std::is_move_assignable<T>::value),
            std::true_type,
            std::false_type
        >;
    template <typename T>
    using prefer_copy_over_move_assignable = 
        std::conditional_t<
            (!std::is_nothrow_move_assignable<T>::value && std::is_nothrow_copy_assignable<T>::value) ||
            (!std::is_move_assignable<T> && std::is_copy_assignable<T>),
            std::true_type,
            std::false_type
        >;
    template <typename T>
    using prefer_trivially_assignable =
        std::conditional_t<
            std::is_trivially_copy_assignable<T>::value ||
            std::is_trivially_move_assignable<T>::value,
            std::true_type,
            std::false_type
        >;
        
    template <typename T>
    using is_copy_or_move_assignable = 
        std::conditional_t<
            std::is_trivially_copy_assignable<T>::value ||
            std::is_trivially_move_assignable<T>::value ||
            std::is_copy_assignable<T>::value ||
            std::is_move_assignable<T>::value,
            std::true_type,
            std::false_type
        >;
    template <typename T>
    using is_move_or_copy_assignable = is_copy_or_move_assignable;
}

#endif