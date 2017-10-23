#ifndef SN_TYPETRAITS_TYPE_TRAITS
#define SN_TYPETRAITS_TYPE_TRAITS

#include "../sn_CommonHeader.h"

namespace sn_TypeTraits {

	template <bool v>
	struct bool_constant_dispatch {
		using type = std::true_type;
	};

	template <>
	struct bool_constant_dispatch<false> {
		using type = std::false_type;
	};

	template <bool v>
	using bcd_t = typename bool_constant_dispatch<v>::type;

    struct is_functor_impl {
        template <typename T>
        static std::true_type check(decltype(&T::operator())*);
        template <typename T>
        static std::false_type check(...);
    };

    template <typename T>
    using is_functor = decltype(is_functor_impl::template check<T>(nullptr));

    template <typename T, bool = std::is_function<std::remove_pointer_t<T>>::value
        || is_functor<T>::value>
        struct is_closure_impl {};

    template <typename T>
    struct is_closure_impl<T, true> : std::true_type {};
    template <typename T>
    struct is_closure_impl<T, false> : std::false_type {};

    template <typename T>
    struct is_closure : is_closure_impl<std::decay_t<T>> {};
	
    template <typename T>
    inline const T* addressofex(const T* t) {
        return t;
    }
    template <typename T>
    inline T* addressofex(T* t) {
        return t;
    }
    template <typename T>
    inline const T* addressofex(const T& t) {
        return std::addressof(t);
    }
    template <typename T>
    inline T* addressofex(T& t) {
        return std::addressof(t);
    }

	// This is just llvm (remember the dynamic_cast)
    template<typename T> char & is_polymorphic_impl(
        std::enable_if_t<
            sizeof(
                (T*) dynamic_cast<const volatile void*> (std::declval<T*>())
            ) != 0, int>
    );
    template<typename T> int &is_polymorphic_impl(...);

    template <class T> 
    struct is_polymorphic : public std::integral_constant<bool, sizeof(is_polymorphic_impl<T>(0)) == 1> {};	

	template <typename T>
	struct is_class_or_union {
		template <typename U>
		static int check(void(U::*)(void));
		static char check(...);
		static const bool value = sizeof(check<T>(0)) == sizeof(int);
	};

	template<typename>
	struct is_decomposable : std::false_type {};

	template<template<typename...> class _gOp, typename... _types>
	struct is_decomposable<_gOp<_types...>> : std::true_type {};

	template <typename T, typename ... V>
	/* inline */ constexpr auto is_brace_constructible_(T*)
		-> decltype(T{std::declval<V>()...}, std::true_type{}) {
		return {};
	}

	template <typename T, typename ... V>
	/* inline */ constexpr std::false_type is_brace_constructible_(...) {
		return {};
	}

	template <typename T, typename ... V>
	/* inline */ constexpr auto is_brace_constructible()
		-> decltype(is_brace_constructible_<T, V...>(nullptr)) {
		return {};
	}

	template <typename T, typename ... V>
	/* inline */ constexpr auto is_paren_constructible_(T*)
		-> decltype(T(std::declval<V>()...), std::true_type{}) {
		return {};
	}

	template <typename T, typename ... V>
	/* inline */ constexpr std::false_type is_paren_constructible_(...) {
		return {};
	}

	template <typename T, typename ... V>
	/* inline */ constexpr auto is_paren_constructible()
		-> decltype(is_paren_constructible_<T, V...>(nullptr)) {
		return {};
	}

	// the ternary operator deduces its type as most specialized type common to both arguments
	template <typename ...Args>
	struct common_type {};

	template <typename T>
	struct common_type<T> {
		using type = T;
	};

	template <typename T, typename U>
	struct common_type<T, U> {
		using type = decltype(true ? std::declval<T>() : std::declval<U>());
	};

	template <typename T, typename U, typename ...Ts>
	struct common_type<T, U, Ts...> {
		using type = typename common_type<typename common_type<T, U>::type, Ts...>::type;
	};

    template <typename ...Args>
    using common_type_t = typename common_type<Args...>::type;

	template <typename T, typename It>
	using is_iterator = bcd_t<std::is_same<T, std::decay_t<decltype(*std::declval<It>())>>::value>;
	
	namespace detail {
		template <typename B, typename D>
		struct Host {
			operator B*() const;
			operator D*() const;
		};
	}

	// @ref: https://stackoverflow.com/questions/2910979/how-does-is-base-of-work
	// Complex overload resolution. Actually if B is base of D, 3/4 will cause same result for 1/2==2
	// Another implementation
	// @ref: http://en.cppreference.com/w/cpp/types/is_base_of
	template <typename B, typename D>
	struct is_base_of {
		using Accept_t = int;
		using Reject_t = char;
		template <typename T>
		static Accept_t check(D*, T);
		static Reject_t check(B*, int);

		constexpr static const bool value = sizeof(check(detail::Host<B, D>{}, int{})) == sizeof(Accept_t);
	};

	template <typename T>
	struct is_function : 
		std::integral_constant<bool, 
			!std::is_object<T>::value &&	// Or: std::is_const<T const>
			!std::is_void<T>::value &&		// libcxx: is_class || is_union || is_void || is_reference || is_nullptr_t
			!std::is_reference::value
		> {};
}