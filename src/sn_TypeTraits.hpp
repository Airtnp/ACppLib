#ifndef SN_TYPE_TRAITS_H
#define SN_TYPE_TRAITS_H

#include "sn_CommonHeader.h"
#include "sn_TypeLisp.hpp"

namespace sn_TypeTraits {
	namespace functor {
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
	}
	
	namespace address {
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
	}
	// This is just llvm (remember the dynamic_cast)
	namespace polymorphic {
		template<typename T> char & is_polymorphic_impl(
			std::enable_if_t<
				sizeof(
					(T*) dynamic_cast<const volatile void*> (std::declval<T*>())
				) != 0, int>
		);
		template<typename T> int &is_polymorphic_impl(...);
		template <class T> 
		struct is_polymorphic : public std::integral_constant<bool, sizeof(is_polymorphic_impl<T>(0)) == 1> {};
	}

	namespace sequence {
		template <std::size_t ...I>
		struct index_sequence {
			using type = index_sequence;
		};

		template <typename T, typename U>
		struct concate {};

		template <std::size_t ...I1, std::size_t ...I2>
		struct concate<index_sequence<I1...>, index_sequence<I2...>>
			: index_sequence<I1..., (I2 + sizeof...(I1))...> {};
		template <std::size_t N>
		struct make_index_sequence_impl
			: concate<
				typename make_index_sequence_impl<N / 2>::type,
				typename make_index_sequence_impl<N - N / 2>::type
			> {};

		template <>
		struct make_index_sequence_impl<0> : index_sequence<> {};
		template <>
		struct make_index_sequence_impl<1> : index_sequence<0> {};

		template <std::size_t N>
		using make_index_sequence = typename make_index_sequence_impl<N>::type;

	}

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

}











#endif