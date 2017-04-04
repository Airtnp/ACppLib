#ifndef SN_TC_H
#define SN_TC_H

#include "sn_CommonHeader.h"

// Type Calculus
namespace sn_TC {
	namespace basic_traits {
		template <std::size_t N1, std::size_t N2>
		struct MaxTraits {
			static constexpr const std::size_t N = N1 > N2 ? N1 : N2;
		};

		template <std::size_t N1, std::size_t N2>
		struct ArrayTraits {
			static constexpr const std::size_t S = N1 > N2 ? N2 : N1;
			static constexpr const std::size_t L = MaxTraits<N1, N2>::N;
		};

		template <std::size_t I, typename T, std::size_t N>
		constexpr T sp_get(const std::array<T, N>& a) {
			return I >= N ? 0 : a[I];
		}

		template <std::size_t N1, std::size_t N2, std::size_t ...I>
		constexpr auto array_add_impl(std::array<std::size_t, N1> arr1, std::array<std::size_t, N2> arr2, std::index_sequence<I...>) {
			return std::array<std::size_t, ArrayTraits<N1, N2>::L>{(sp_get<I>(arr1) + sp_get<I>(arr2))...};
		}

		template <std::size_t N1, std::size_t N2>
		constexpr auto array_add(std::array<std::size_t, N1> arr1, std::array<std::size_t, N2> arr2) {
			return array_add_impl(arr1, arr2, std::make_index_sequence<ArrayTraits<N1, N2>::L>{});
		}

		
	}

	namespace basic_types {
		struct Zero {
			using T = Zero;
			using D = Zero;
			static constexpr const std::size_t N = 0;
			static constexpr const std::array<std::size_t, N + 1> A = { 0 };
		};
		
		struct One {
			using T = One;
			using D = Zero;
			static constexpr const std::size_t N = 0;
			static constexpr const std::array<std::size_t, N + 1> A = { 1 };
		};

		struct Unit {
			using T = Unit;
			using D = One;
			static constexpr const std::size_t N = 1;
			static constexpr const std::array<std::size_t, N + 1> A = { 0, 1 };
		};

		
		using Void = Zero;
		using X = Unit;

		template <typename T, typename U>
		struct Multiply {};
		template <typename T, typename U>
		struct Add {};
		template <typename T, typename U>
		struct Equal {};

		template <typename U>
		struct Add<Void, U> {
			using T = U;
			static constexpr const auto A = U::A;
		};




		template <typename V, typename U>
		struct Union {
			using T = Union<V, U>;
			using D = Add<Multiply<typename V::D, U>, Multiply<V, typename U::D>>;
		};
		
		template <typename V, typename U>
		struct Either {
			using T = Either<V, U>;
			using D = Add<typename V::D, typename U::D>;
		};
		
		template <typename T>
		struct Eval {};

		template <typename T>
		struct Derivative {};

	}
}



#endif