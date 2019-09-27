#ifndef SN_CONCEPT_BUILTIN_H
#define SN_CONCEPT_BUILTIN_H

#include "../sn_CommonHeader.h"
#include "../sn_TypeTraits.hpp"

/// PascalCase re-implementation of @ref: https://en.cppreference.com/w/cpp/header/concepts
/// @note: Concepts are auto-curried
/// @todo: add range concepts && other new concepts
namespace sn_Concept {
    namespace detail {
        template<class T, class U>
        concept SameOne = std::is_same_v<T, U>;
    }
    template <class T, class U>
    concept Same = detail::SameOne<T, U> && detail::SameOne<U, T>;

    /// Base is a class type that is either Derived or a public and unambiguous base od Derived, ignoring cv-qualifiers
    /// When Base is a private/protected base od Derived, is_base_of will be true, but convertiable will be false
    template <class Derived, class Base>
    concept DerivedFrom = std::is_base_of_v<Base, Derived>
            && std::is_convertible_v<const volatile Derived*, const volatile Base*>;

    /// @ref: https://en.cppreference.com/w/cpp/concepts/convertible_to
    /// for equality preservation
    template <class From, class To>
    concept ConvertibleTo = std::is_convertible_v<From, To>
            && requires(From(&f)()) {
                static_cast<To>(f());
    };

#ifdef SN_ENABLE_CXX2A_FEATURE
    template <class T, class U>
    concept CommonReference =
            Same<std::common_reference_t<T, U>, std::common_reference_t<U, T>> &&
            ConvertibleTo<T, std::common_reference_t<T, U>> &&
            ConvertibleTo<U, std::common_reference_t<T, U>>;

    template <class T, class U>
    concept Common =
            Same<std::common_type_t<T, U>, std::common_type_t<U, T>> &&
            requires {
                    static_cast<std::common_type_t<T, U>>(std::declval<T>());
                    static_cast<std::common_type_t<T, U>>(std::declval<U>());
            } &&
            CommonReference<
                    std::add_lvalue_reference_t<const T>,
                    std::add_lvalue_reference_t<const U>> &&
            CommonReference<
                    std::add_lvalue_reference_t<std::common_type_t<T, U>>,
                    std::common_reference_t<
                            std::add_lvalue_reference_t<const T>,
                            std::add_lvalue_reference_t<const U>>>;
#else
    template <class T, class U>
    concept CommonReference = Same<T, U>;

    template <class T, class U>
    concept Common = Same<std::common_type_t<T, U>, std::common_type_t<U, T>> &&
            requires {
                    static_cast<std::common_type_t<T, U>>(std::declval<T>());
                    static_cast<std::common_type_t<T, U>>(std::declval<U>());
            };
#endif


    template <class LHS, class RHS>
    concept Assignable =
            std::is_lvalue_reference_v<LHS> &&
            CommonReference<const std::remove_reference_t<LHS>&, const std::remove_reference_t<RHS>&> &&
            requires(LHS lhs, RHS&& rhs) {
                { lhs = std::forward<RHS>(rhs) } -> Same<LHS>
            };

    template <class T>
    concept Integral = std::is_integral_v<T>;

    template <class T>
    concept SignedIntegral = Integral<T> && std::is_signed_v<T>;

    template <class T>
    concept UnsignedIntegral = Integral<T> && std::is_unsigned_v<T>;

    template <class T>
    concept FloatingPoint = std::is_floating_point_v<T>;

    template <class T>
    concept Swappable = requires(T& a, T& b) {
        // actually ranges::swap(a, b)
        std::swap(a, b);
    };

    template <class T, class U>
    concept SwappableWith =
            CommonReference<const std::remove_reference_t<T>&, const std::remove_reference_t<U>&> &&
            requires(T&& t, U&& u) {
                // all ranges::swap
                std::swap(std::forward<T>(t), std::forward<T>(t));
                std::swap(std::forward<T>(t), std::forward<U>(u));
                std::swap(std::forward<U>(u), std::forward<U>(u));
                std::swap(std::forward<U>(u), std::forward<T>(t));
            };

    template <class T>
    concept Destructible = std::is_nothrow_destructible_v<T>;

    template <class T, class ...Args>
    concept ConstructibleFrom = Destructible<T> && std::is_constructible_v<T, Args...>;

    template <class T>
    concept DefaultConstructible = ConstructibleFrom<T>;

    template <class T>
    concept MoveConstructible = ConstructibleFrom<T, T> && ConvertibleTo<T, T>;

    template <class T>
    concept CopyConstructible =
            MoveConstructible<T> &&
            ConstructibleFrom<T, T&> && ConvertibleTo<T&, T> &&
            ConstructibleFrom<T, const T&> && ConvertibleTo<const T&, T> &&
            ConstructibleFrom<T, const T> && ConvertibleTo<const T, T>;

    /// object that can moved (move constructed, move assigned, lvalue of T can be swapped)
    template <class T>
    concept Movable =
            std::is_object_v<T> &&
            MoveConstructible<T> &&
            Assignable<T&, T> &&
            Swappable<T>;

    // movable object that can be copied (copy construction, copy assignment)
    template <class T>
    concept Copyable =
            CopyConstructible<T> &&
            Movable<T> &&
            Assignable<T&, const T&>;

    template <class T>
    concept SemiRegular = Copyable<T> && DefaultConstructible<T>;

    template <class B>
    concept Boolean =
            Movable<sn_TypeTraits::remove_cvref_t<B>> &&
                    requires(const std::remove_reference_t<B>& b1,
                             const std::remove_reference_t<B>& b2, const bool a) {
                        { b1 } -> ConvertibleTo<bool>;
                        { !b1 } -> ConvertibleTo<bool>;
                        { b1 && b2 } -> Same<bool>;
                        { b1 &&  a } -> Same<bool>;
                        {  a && b2 } -> Same<bool>;
                        { b1 || b2 } -> Same<bool>;
                        { b1 ||  a } -> Same<bool>;
                        {  a || b2 } -> Same<bool>;
                        { b1 == b2 } -> ConvertibleTo<bool>;
                        { b1 ==  a } -> ConvertibleTo<bool>;
                        {  a == b2 } -> ConvertibleTo<bool>;
                        { b1 != b2 } -> ConvertibleTo<bool>;
                        { b1 !=  a } -> ConvertibleTo<bool>;
                        {  a != b2 } -> ConvertibleTo<bool>;
                    };

    template <class T, class U>
    concept WeaklyEqualityComparableWith =
            requires(const std::remove_reference_t<T>& t,
                     const std::remove_reference_t<U>& u) {
                { t == u } -> Boolean;
                { t != u } -> Boolean;
                { u == t } -> Boolean;
                { u != t } -> Boolean;
            };

    template <class T>
    concept EqualityComparable = WeaklyEqualityComparableWith<T, T>;

#ifdef SN_ENABLE_CXX2A_FEATURE
    template <class T, class U>
    concept EqualityComparableWith =
            EqualityComparable<T> &&
            EqualityComparable<U> &&
            CommonReference<
                    const std::remove_reference_t<T>&,
                    const std::remove_reference_t<U>&> &&
            EqualityComparable<
                    std::common_reference_t<
                            const std::remove_reference_t<T>&,
                            const std::remove_reference_t<U>&>> &&
            WeaklyEqualityComparableWith<T, U>;
#else
    template <class T, class U>
    concept EqualityComparableWith =
            EqualityComparable<T> &&
            EqualityComparable<U> &&
            CommonReference<
                    const std::remove_reference_t<T>&,
                    const std::remove_reference_t<U>&> &&
            WeaklyEqualityComparableWith<T, U>;
#endif

    template <class T>
    concept StrictTotallyOrdered =
            EqualityComparable<T> &&
            requires(const std::remove_reference_t<T>& a,
                     const std::remove_reference_t<T>& b) {
                { a <  b } -> Boolean;
                { a >  b } -> Boolean;
                { a <= b } -> Boolean;
                { a >= b } -> Boolean;
            };

#ifdef SN_ENABLE_CXX2A_FEATURE

template <class T, class U>
    concept StrictTotallyOrderedWith =
            StrictTotallyOrdered<T> &&
            StrictTotallyOrdered<U> &&
            CommonReference<
                    const std::remove_reference_t<T>&,
                    const std::remove_reference_t<U>&> &&
            StrictTotallyOrdered<
                    std::common_reference_t<
                            const std::remove_reference_t<T>&,
                            const std::remove_reference_t<U>&>> &&
            EqualityComparableWith<T, U> &&
            requires(const std::remove_reference_t<T>& t,
                     const std::remove_reference_t<U>& u) {
                { t <  u } -> Boolean;
                { t >  u } -> Boolean;
                { t <= u } -> Boolean;
                { t >= u } -> Boolean;
                { u <  t } -> Boolean;
                { u >  t } -> Boolean;
                { u <= t } -> Boolean;
                { u >= t } -> Boolean;
            };
#else
    template <class T, class U>
    concept StrictTotallyOrderedWith =
            StrictTotallyOrdered<T> &&
            StrictTotallyOrdered<U> &&
            CommonReference<
                    const std::remove_reference_t<T>&,
                    const std::remove_reference_t<U>&> &&
            EqualityComparableWith<T, U> &&
            requires(const std::remove_reference_t<T>& t,
                     const std::remove_reference_t<U>& u) {
                { t <  u } -> Boolean;
                { t >  u } -> Boolean;
                { t <= u } -> Boolean;
                { t >= u } -> Boolean;
                { u <  t } -> Boolean;
                { u >  t } -> Boolean;
                { u <= t } -> Boolean;
                { u >= t } -> Boolean;
            };
#endif



    template <class T>
    concept Regular = SemiRegular<T> && EqualityComparable<T>;

    template <class F, class ...Args>
    concept Invocable = requires(F&& f, Args&&... args) {
        std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    };

    // Rng generator could be non-equality-preserving, which breaks the RegularInvocable
    template <class F, class ...Args>
    concept RegularInvocable = Invocable<F, Args...>;

    template <class F, class ...Args>
    concept Predicate = RegularInvocable<F, Args...> && Boolean<std::invoke_result_t<F, Args...>>;

    /// binary relationship R between T & U
    template <class R, class T, class U>
    concept Relation =
            Predicate<R, T, T> && Predicate<R, U, U> &&
            Predicate<R, T, U> && Predicate<R, U, T>;

    /// irreflexive: forall x. r(x, x) is false
    /// transitive: forall a, b, c. r(a, b) && r(b, c) => r(a, c)
    /// transitive of incomparability: e(a, b) := !r(a, b) && !r(b, a). e(a, b) is transitive
    template <class R, class T, class U>
    concept StrictWeakOrder = Relation<R, T, U>;
}


#endif