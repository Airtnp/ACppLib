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

		template <typename T, T ...Is>
		struct integer_pack : std::integral_constant<std::size_t, sizeof...(Is)> {
			static_assert(std::is_integral<T>::value, "integer_pack can only contain integral value.");
			using value_type = T;
			using type = integer_pack<T, Is...>;
			static constexpr const std::size_t size = sizeof...(Is); // or std::integral_constant<std::size_t, sizeof...(Is)>::value
			static constexpr const std::array<T, sizeof...(Is)> elements = {Is...};
		};

		template <std::size_t ...Is>
		using index_pack = integer_pack<std::size_t, Is...>;

		template <typename T>
		struct integer_pack_size {};

		template <typename T, T ...Is>
		struct integer_pack_size<integer_pack<T, Is...>>
			: std::integral_constant<std::size_t, sizeof...(Is)> {};

		template <typename T>
		constexpr const std::size_t integer_pack_size_v = integer_pack_size<T>::value;

		template <typename T>
		struct integer_pack_negate {};

		template <typename T, T ...Is>
		struct integer_pack_negate<integer_pack<T, Is...>>
			: integer_pack<T, (-Is)...> {};

		template <typename T>
		using integer_pack_negate_t = typename integer_pack_negate<T>::type;

		template <typename T, typename P1, typename P2>
		struct integer_pack_merge {};

		template <typename T, T ...Is1, T ...Is2>
		struct integer_pack_merge<T, integer_pack<T, Is1...>, integer_pack<T, Is2...>>
			: integer_pack<T, Is1..., Is2...> {};
		
		template <typename T, typename P1, typename P2>
		using integer_pack_merge_t = typename integer_pack_merge<T, P1, P2>::type;

		template <typename T, typename P1, typename P2>
		struct integer_pack_concate {};

		template <typename T, T ...Is1, T ...Is2>
		struct integer_pack_concate<T, integer_pack<T, Is1...>, integer_pack<T, Is2...>>
			: integer_pack<T, I1..., (I2 + sizeof...(I1))...> {};
		template <typename T, T N>
		struct make_integer_pack_impl
			: integer_pack_concate<
				typename make_integer_pack_impl<T, N / 2>::type,
				typename make_integer_pack_impl<T, N - N / 2>::type
			> {};

		template <typename T>
		struct make_integer_pack_impl<T, 0> : integer_pack<T> {};
		template <typename T>
		struct make_integer_pack_impl<T, 1> : integer_pack<T, 0> {};

		template <typename T, T N>
		using make_integer_pack = typename make_integer_pack_impl<T, N>::type;

		template <std::size_t N>
		using make_index_pack = typename make_integer_pack_impl<std::size_t, N>::type;

		template <typename T, T from, T to, T step, T nvals = (from < to ? to - from : from - to)>
		struct make_integer_pack_range_impl {
		private:
			static_assert(nvals % step == 0, "bad step value.");
			template <typename T, bool dir>
			struct make_integer_pack_range_assist {};

			template <T ...Is>
			struct make_integer_pack_range_assist<integer_pack<T, Is...>, true>
				: integer_pack<T, (from + step * Is)...> {};

			template <T ...Is>
			struct make_integer_pack_range_assist<integer_pack<T, Is...>, false>
				: integer_pack<T, (from - step * Is)...> {};
		public:
			using type = typename make_integer_pack_range_assist<
				make_integer_pack<T, 1 + nvals / step>, (from < to)
			>::type;
		};

		template <typename T, T n, T step, T nvals>
		struct make_integer_pack_range_impl<T, n, n, step, nvals>
			: integer_pack<T, n> {};

		template <typename T, T from, T to, T step>
		using make_integer_pack_range = typename make_integer_pack_range_impl<T, from, to, step>::type;

		template <std::size_t from, std::size_t to, std::size_t step>
		using make_index_pack_range = typename make_integer_pack_range_impl<std::size_t, from, to, step>::type;

		template <std::size_t I, typename T>
		struct integer_pack_at {};

		template <std::size_t I, typename T, T ...Is>
		struct integer_pack_at<I, integer_pack<T, Is...>> {
			static_assert(sizeof...(Is) != 0, "No element.");
			static_assert(I < sizeof...(Is), "Index out of bounds.");
			constexpr static const std::array<T, sizeof...(Is)> elements = {Is...};
			constexpr static const T value = elements[I];
		};

		// https://codereview.stackexchange.com/questions/133626/integer-packs-and-integer-pack-utilities-for-template-meta-programming
		// TODO: add shift/sort/max/min/union
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

	enum class type_qualifier {
		value,
		const_value,
		volatile_value,
		volatile_const_value,
		lref,
		rref,
		const_lref,
		const_rref,
		volatile_lref,
		volatile_rref,
		volatile_const_lref,
		volatile_const_rref,

		count_
	};

	template< type_qualifier type_qual, typename type > struct add_type_qualifier;
	template< typename to > struct add_type_qualifier< type_qualifier::value                , to > { using type =          to         ; };
	template< typename to > struct add_type_qualifier< type_qualifier::const_value          , to > { using type =          to const   ; };
	template< typename to > struct add_type_qualifier< type_qualifier::volatile_value       , to > { using type = volatile to         ; };
	template< typename to > struct add_type_qualifier< type_qualifier::volatile_const_value , to > { using type = volatile to const   ; };
	template< typename to > struct add_type_qualifier< type_qualifier::lref                 , to > { using type =          to       & ; };
	template< typename to > struct add_type_qualifier< type_qualifier::rref                 , to > { using type =          to       &&; };
	template< typename to > struct add_type_qualifier< type_qualifier::const_lref           , to > { using type =          to const & ; };
	template< typename to > struct add_type_qualifier< type_qualifier::const_rref           , to > { using type =          to const &&; };
	template< typename to > struct add_type_qualifier< type_qualifier::volatile_lref        , to > { using type = volatile to       & ; };
	template< typename to > struct add_type_qualifier< type_qualifier::volatile_rref        , to > { using type = volatile to       &&; };
	template< typename to > struct add_type_qualifier< type_qualifier::volatile_const_lref  , to > { using type = volatile to const & ; };
	template< typename to > struct add_type_qualifier< type_qualifier::volatile_const_rref  , to > { using type = volatile to const &&; };

	template< type_qualifier type_qual, typename to >
	using add_type_qualifier_t = typename add_type_qualifier< type_qual, to >::type;

	template< typename from > constexpr type_qualifier type_qualifier_of                           = type_qualifier::value                ;
	template< typename from > constexpr type_qualifier type_qualifier_of<          from const    > = type_qualifier::const_value          ;
	template< typename from > constexpr type_qualifier type_qualifier_of< volatile from          > = type_qualifier::volatile_value       ;
	template< typename from > constexpr type_qualifier type_qualifier_of< volatile from const    > = type_qualifier::volatile_const_value ;
	template< typename from > constexpr type_qualifier type_qualifier_of<          from       &  > = type_qualifier::lref                 ;
	template< typename from > constexpr type_qualifier type_qualifier_of<          from       && > = type_qualifier::rref                 ;
	template< typename from > constexpr type_qualifier type_qualifier_of<          from const &  > = type_qualifier::const_lref           ;
	template< typename from > constexpr type_qualifier type_qualifier_of<          from const && > = type_qualifier::const_rref           ;
	template< typename from > constexpr type_qualifier type_qualifier_of< volatile from       &  > = type_qualifier::volatile_lref        ;
	template< typename from > constexpr type_qualifier type_qualifier_of< volatile from       && > = type_qualifier::volatile_rref        ;
	template< typename from > constexpr type_qualifier type_qualifier_of< volatile from const &  > = type_qualifier::volatile_const_lref  ;
	template< typename from > constexpr type_qualifier type_qualifier_of< volatile from const && > = type_qualifier::volatile_const_rref  ;

	template< typename from, typename to >
	using copy_cv_reference_t = add_type_qualifier_t< type_qualifier_of< from >, to >;

}











#endif