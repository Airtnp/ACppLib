//
// Created by xiaol on 11/29/2019.
//

#ifndef ACPPLIB_ALGORITHM_HPP
#define ACPPLIB_ALGORITHM_HPP

#include "../sn_CommonHeader.h"
#include "sn_Concept/builtin.hpp"
#include "sn_Concept/helper.hpp"

namespace sn_Concept {
    /// @ref: CppCon2019: 6 algorithmic journeys with concepts
    /// @note: slightly different from std, separate file
    namespace Alg {
        template <typename ...Args>
        struct all_same;

        template <typename T, typename U, typename ...Ts>
        struct all_same<T, U, Ts...> : std::conditional_t<
                std::is_same_v<T, U>,
                all_same<U, Ts...>,
                std::false_type
            > {};

        template <typename T, typename U>
        struct all_same<T, U> : std::is_same<T, U> {};

        template <typename ...Args>
        inline constexpr bool all_same_v = all_same<Args...>::value;

        template <typename T>
        concept is_equality_comparable = EqualityComparable<T>;

        template <typename T>
        concept semiregular =
                std::is_default_constructible_v<T> &&
                std::is_copy_constructible_v<T> &&
                std::is_copy_assignable_v<T> &&
                std::is_destructible_v<T>;

        /// copy construction
        /// assignemnt
        /// equality
        /// destruction
        template <typename T>
        concept regular = semiregular<T> && is_equality_comparable<T>;

        SN_CONCEPT_CAST(is_regular, regular, 1);

        template <typename F, typename ...T>
        concept functional_procedure =
                (regular<std::invoke_result_t<F, T...>> || std::is_same_v<std::invoke_result_t<F, T...>, void>) &&
                std::conjunction_v<is_regular<T>...>;

        template <typename F, typename T>
        concept unary_function = functional_procedure<F, T> && regular<T>;

        template <typename F, typename ...T>
        concept homogeneous_function =
                functional_procedure<F, T...> &&
                sizeof...(T) > 0 &&
                std::conjunction_v<is_regular<T>...> &&
                all_same<T...>::value;

        template <typename F, typename ...T>
        concept predicate = functional_procedure<F, T...> &&
                std::is_same_v<std::invoke_result_t<F, T...>, bool>;

        template <typename F, typename ...T>
        concept homogeneous_predicate = predicate<F, T...> && homogeneous_function<F, T...>;

        template <typename R, typename T>
        concept relation = predicate<R, T, T>;

        template <typename T>
        concept totally_ordered = StrictTotallyOrdered<T>;

        template <typename T, unary_function<T> Projection>
            requires totally_ordered<std::invoke_result_t<Projection, T>>
        T& min(T& x, T& y, Projection projection) {
            if (projection(y) < projection(x)) {
                return y;
            }
            return x;
        }

        template <typename T>
        concept iterator =
                std::is_same_v<std::forward_iterator_tag, typename std::iterator_traits<T>::iterator_category> ||
                std::is_same_v<std::input_iterator_tag, typename std::iterator_traits<T>::iterator_category> ||
                std::is_same_v<std::output_iterator_tag, typename std::iterator_traits<T>::iterator_category> ||
                std::is_same_v<std::random_access_iterator_tag, typename std::iterator_traits<T>::iterator_category> ||
                std::is_same_v<std::bidirectional_iterator_tag, typename std::iterator_traits<T>::iterator_category>
        ;

        template <typename I>
        concept forward_iterator = iterator<I> &&
                std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<I>::iterator_category>
        ;

        template <typename I>
        concept bidirectional_iterator = iterator<I> &&
                std::is_base_of_v<std::bidirectional_iterator_tag, typename std::iterator_traits<I>::iterator_category>
        ;

        template <typename I>
        concept random_access_iterator = iterator<I> &&
                 std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<I>::iterator_category>
        ;

        template <typename T>
        concept readable = std::is_same_v<decltype(*std::declval<T>()), typename T::value_type&> ||
                std::is_same_v<decltype(*std::declval<T>()), typename T::value_type const&>
        ;

        template <typename T, typename U = typename T::value_type>
        concept writable = requires(T it, U x) {
            *it = x;
        };

        template <typename T>
        concept additive_semigroup = regular<T> && std::is_same_v<decltype(T() + T()), T>;

        template <typename T>
        concept additive_monoid = additive_semigroup<T>; // + having identity element

        template <forward_iterator It, relation<typename It::value_type> R>
            requires readable<It> && writable<It>
        It unique(It first, It last, R r) {
            if (first == last) return last;
            It result = first; ++first;
            while (first != last) {
                if (r(*result, *first)) ++first;
                else {
                    ++result;
                    *result = *first;
                    ++first;
                }
            }
            ++result;
            return result;
        }
    }
}

#endif //ACPPLIB_ALGORITHM_HPP
