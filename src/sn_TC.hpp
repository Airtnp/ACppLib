#ifndef SN_TC_H
#define SN_TC_H

#include "sn_CommonHeader.h"

// Type Calculus (Algebraic Data Type)
// TODO: support recursive(?), add more Derivative
// Note: This only compiles in gcc/clang (tested in 6.3.0/3.9.0) (MSVC failed in ArrayMulOuter unpack Is)
// Note: LimitTree<N> will generate many coefficients, but only first N+1 is valid
namespace sn_TC {
	namespace basic_traits {

		template <std::size_t ...I>
		struct MaxTraits {};

		template <std::size_t I1>
		struct MaxTraits<I1> {
			static constexpr const std::size_t N = I1;
		};

		template <std::size_t I1, std::size_t ...I>
		struct MaxTraits<I1, I...> {
			static constexpr const std::size_t N = I1 > MaxTraits<I...>::N ? I1 : MaxTraits<I...>::N;
		};

		template <std::size_t ...I>
		struct MinTraits {};

		template <std::size_t I1>
		struct MinTraits<I1> {
			static constexpr const std::size_t N = I1;
		};

		template <std::size_t I1, std::size_t ...I>
		struct MinTraits<I1, I...> {
			static constexpr const std::size_t N = I1 > MinTraits<I...>::N ? MinTraits<I...>::N : I1;
		};

		template <std::size_t N1, std::size_t N2>
		struct ArrayTraits {
			static constexpr const std::size_t S = N1 > N2 ? N2 : N1;
			static constexpr const std::size_t L = MaxTraits<N1, N2>::N;
		};

		template <typename T, typename U>
		struct TwoArray {
			static constexpr const std::size_t N1 = T::N + 1; // Array-size instead of rank
			static constexpr const std::size_t N2 = U::N + 1;
			static constexpr const std::array<std::size_t, N1> A1 = T::A;
			static constexpr const std::array<std::size_t, N2> A2 = U::A;
			
		};

		template <std::size_t I, typename T, std::size_t N>
		constexpr T sp_get(const std::array<T, N>& a) {
			return I >= N ? 0 : a[I];
		}

		template <typename T, typename U, std::size_t ...I>
		constexpr auto ArrayAddImpl(std::index_sequence<I...>) {
			return std::array<std::size_t, ArrayTraits<TwoArray<T, U>::N1, TwoArray<T, U>::N2>::L>{(sp_get<I>(TwoArray<T, U>::A1) + sp_get<I>(TwoArray<T, U>::A2))...};
		}

		template <typename T, typename U>
		constexpr auto ArrayAdd() {
			return ArrayAddImpl<T, U>(std::make_index_sequence<ArrayTraits<TwoArray<T, U>::N1, TwoArray<T, U>::N2>::L>{});
		}
		
		// Sum(Arr1[i]*Arr2) Failed
		// Sum(Arr1[i]*Arr2[j]) convolution Failed
		/*
		template <std::size_t I1>
		constexpr auto ArrayAddMul(std::array<std::size_t, I1> arr1) {
			return arr1;
		}

		template <std::size_t I1, std::size_t ...I>
		constexpr auto ArrayAddMul(std::array<std::size_t, I1> arr1, std::array<std::size_t, I>... arr) {
			return ArrayAdd(arr1, ArrayAddMul<I...>(arr...));
		}

		

		template <std::size_t K, std::size_t N, std::size_t ...I>
		constexpr auto ArrayMulScalar(std::array<std::size_t, N> arr, std::index_sequence<I...>) {
			return std::array<std::size_t, N>{ (K * arr[I])...};
		}

		template <std::size_t N1, std::size_t N2, std::size_t ...I>
		constexpr auto ArrayMulImpl(const std::array<std::size_t, N1> arr1, std::array<std::size_t, N2> arr2, std::index_sequence<I...>) {
			return ArrayAddMul((ArrayMulScalar<sp_get<I>(arr1)>(arr2, std::make_index_sequence<N2>{}))...);
		}

		template <std::size_t N1, std::size_t N2>
		constexpr auto ArrayMul(std::array<std::size_t, N1> arr1, std::array<std::size_t, N2> arr2) {
			return ArrayMulImpl(arr1, arr2, std::make_index_sequence<N1>{});
		}*/

		template <std::size_t N, std::size_t ...I>
		constexpr auto make_convolution_tuple_inner(std::index_sequence<I...>) {
			return std::tuple_cat(std::make_tuple(std::tuple<std::size_t, std::size_t>{I, N - I})...);
		}

		template <std::size_t N1, std::size_t ...I>
		constexpr auto make_convolution_tuple_outer(std::index_sequence<I...>) {
			return std::tuple_cat((make_convolution_tuple_inner<I>(std::make_index_sequence<MinTraits<N1, I + 1>::N>{}))...);
		}

		template <std::size_t N1, std::size_t N2>
		constexpr auto make_convolution_tuple() {
			return make_convolution_tuple_outer<N1>(std::make_index_sequence<N1 + N2 - 1>{});
		}

		template <std::size_t N1, std::size_t ...I>
		constexpr auto make_convolution_tuple_helper_impl(std::index_sequence<I...>) {
			return std::index_sequence<MinTraits<N1, I + 1>::N...>{};
		}

		template <std::size_t N1, std::size_t N2>
		constexpr auto make_convolution_tuple_helper() {
			return make_convolution_tuple_helper_impl<N1>(std::make_index_sequence<N1 + N2 - 1>{});
		}
		
		template <std::size_t ...I>
		struct ArrayMulImplInner {};

		template <std::size_t I1, std::size_t ...I>
		struct ArrayMulImplInner<I1, I...> {
			static constexpr const std::size_t N = I1 + ArrayMulImplInner<I...>::N;
		};

		template <std::size_t I1>
		struct ArrayMulImplInner<I1> {
			static constexpr const std::size_t N = I1;
		};

		template <std::size_t II, typename T, typename U, std::size_t ...I>
		constexpr auto ArrayMulImplOuter(std::index_sequence<I...> idx_seq) {
			return ArrayMulImplInner<
				sp_get<std::get<0>(std::get<I>(
					make_convolution_tuple_inner<II>(
						idx_seq
						)
					))>(TwoArray<T, U>::A1) *
				sp_get<std::get<1>(std::get<I>(
					make_convolution_tuple_inner<II>(
						idx_seq
						)
					))>(TwoArray<T, U>::A2)...>::N;
		}

		template <typename T, typename U, std::size_t ...I>
		constexpr auto ArrayMulImpl(std::index_sequence<I...>) {
			return std::array<
				std::size_t,
				TwoArray<T, U>::N1 + TwoArray<T, U>::N2 - 1>
			{(
				ArrayMulImplOuter<I, T, U>(
					std::make_index_sequence<MinTraits<TwoArray<T, U>::N1, I + 1>::N>{})
				)...};
		}

		template <typename T, typename U>
		constexpr auto ArrayMul() {
			return ArrayMulImpl<T, U>(std::make_index_sequence<TwoArray<T, U>::N1 + TwoArray<T, U>::N2 - 1>{});
		}

		template <std::size_t N1, typename T>
		struct ArrayPowerImpl {
			constexpr static const std::size_t N = T::N;
			constexpr static const std::array<std::size_t, N1 * N + 1> A = ArrayMul<T, ArrayPowerImpl<N - 1, T>>();
		};

		template <typename T>
		struct ArrayPowerImpl<1, T> {
			constexpr static const std::size_t N = T::N;
			constexpr static const std::array<std::size_t, N + 1> A = T::A;
		};

		template <typename T>
		struct ArrayPowerImpl<0, T> {
			constexpr static const std::size_t N = 0;
			constexpr static const std::array<std::size_t, N + 1> A = { 1 };
		};


		template <typename T, typename U>
		constexpr auto ArrayPower() {
			return ArrayPowerImpl<std::get<0>(U::A), T>::A;
		}
		
	}

	namespace basic_types {

		using namespace basic_traits;

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

		template <std::size_t N1>
		struct Number {
			using T = Number<N1>;
			using D = Zero;
			static constexpr const std::size_t N = 0;
			static constexpr const std::array<std::size_t, N + 1> A = { N1 };
		};

		struct TypeX {
			using T = TypeX;
			using D = One;
			static constexpr const std::size_t N = 1;
			static constexpr const std::array<std::size_t, N + 1> A = { 0, 1 };
		};

		
		using Void = Zero;
		using Unit = One;

		template <typename V, typename U>
		struct Add;

		// T * U / (T, U)
		template <typename V, typename U>
		struct Multiply {
			using T = Multiply<V, U>;
			using D1 = typename Multiply<typename V::D, U>::T;
			using D2 = typename Multiply<V, typename U::D>::T;
			using D = typename Add<D1, D2>::T;
			static constexpr const std::size_t N = V::N + U::N;
			static constexpr const auto A = ArrayMul<V, U>();
		};
		// T + U / T | U
		template <typename V, typename U>
		struct Add {
			using T = Add<V, U>;
			using D = typename Add<typename V::D, typename U::D>::T;
			static constexpr const std::size_t N = MaxTraits<V::N, U::N>::N;
			static constexpr const std::array<std::size_t, N + 1> A = ArrayAdd<V, U>();
		};
		template <typename V, typename U>
		struct Equal {};
		template <typename V>
		struct Derivative {
			using T = typename V::D;
			using D = typename T::D;
			static constexpr const std::size_t N = MaxTraits<V::N, 1>::N - 1;  //avoid overflow
			static constexpr const std::array<std::size_t, N + 1> A = V::D::A;
		};
		// U^T / T -> U
		template <typename V, typename U>
		struct Power {
			using T = Power<V, U>;
			using D = typename Multiply<typename Number<std::get<0>(U::A)>::T, typename Power<V, Number<std::get<0>(U::A) - 1>>::T>::T;
			static constexpr const std::size_t N = std::get<0>(U::A) * V::N;
			static constexpr const std::array<std::size_t, N + 1> A = ArrayPower<V, U>();
		};
		


		template <>
		struct Add<Void, Void> {
			using T = Void;
			using D = typename T::D;
			static constexpr const std::size_t N = T::N;
			static constexpr const std::array<std::size_t, N + 1> A = T::A;
		};

		template <typename U>
		struct Add<Void, U> {
			using T = U;
			using D = typename T::D;
			static constexpr const std::size_t N = T::N;
			static constexpr const std::array<std::size_t, N + 1> A = T::A;
		};

		template <typename U>
		struct Add<U, Void> {
			using T = U;
			using D = typename T::D;
			static constexpr const std::size_t N = T::N;
			static constexpr const std::array<std::size_t, N + 1> A = T::A;
		};

		template <>
		struct Multiply<Void, Void> {
			using T = Void;
			using D = typename T::D;
			static constexpr const std::size_t N = T::N;
			static constexpr const std::array<std::size_t, N + 1> A = T::A;
		};

		template <typename U>
		struct Multiply<Void, U> {
			using T = Void;
			using D = typename T::D;
			static constexpr const std::size_t N = T::N;
			static constexpr const std::array<std::size_t, N + 1> A = T::A;
		};

		template <typename U>
		struct Multiply<U, Void> {
			using T = Void;
			using D = typename T::D;
			static constexpr const std::size_t N = T::N;
			static constexpr const std::array<std::size_t, N + 1> A = T::A;
		};

		template <>
		struct Multiply<One, One> {
			using T = One;
			using D = typename T::D;
			static constexpr const std::size_t N = T::N;
			static constexpr const std::array<std::size_t, N + 1> A = T::A;
		};

		template <typename U>
		struct Multiply<One, U> {
			using T = U;
			using D = typename T::D;
			static constexpr const std::size_t N = T::N;
			static constexpr const std::array<std::size_t, N + 1> A = T::A;
		};

		template <typename U>
		struct Multiply<U, One> {
			using T = U;
			using D = typename T::D;
			static constexpr const std::size_t N = T::N;
			static constexpr const std::array<std::size_t, N + 1> A = T::A;
		};


		template <>
		struct Power<One, One> {
			using T = One;
			using D = typename T::D;
			static constexpr const std::size_t N = T::N;
			static constexpr const std::array<std::size_t, N + 1> A = T::A;
		};

		template <typename U>
		struct Power<One, U> {
			using T = U;
			using D = typename T::D;
			static constexpr const std::size_t N = T::N;
			static constexpr const std::array<std::size_t, N + 1> A = T::A;
		};

		template <typename U>
		struct Power<U, One> {
			using T = One;
			using D = typename T::D;
			static constexpr const std::size_t N = T::N;
			static constexpr const std::array<std::size_t, N + 1> A = T::A;
		};

		template <typename V, typename U>
		using Union = Multiply<V, U>;

		template <typename V, typename U>
		using Either = Add<V, U>;

		template <typename V, typename U>
		using Map = Power<V, U>;

		

		/*
		template <typename V, typename U>
		struct Union {
			using T = Add<V, U>;
			using D = Add<Multiply<typename V::D, U>, Multiply<V, typename U::D>>;

		};
		
		template <typename V, typename U>
		struct Either {
			using T = Either<V, U>;
			using D = Add<typename V::D, typename U::D>;
		};*/
		


		template <typename T>
		struct Eval {};

		
	}

	namespace ADT {
		using namespace basic_types;
		struct List {
			using Node = TypeX;
			using T = Either<Unit, Union<Node, List>>;
			using D = Derivative<T>;
		};

		struct Pair {
			using T = Union<TypeX, TypeX>;
			using D = Derivative<T>;
		};

		struct Bool {
			using T = Either<Unit, Unit>;
			using D = Derivative<T>;
		};

		struct BinaryTree {
			using T = Either<Unit, Union<Union<TypeX, BinaryTree>, BinaryTree>>;
			using D = Derivative<T>;
		};

		template <std::size_t N1>
		struct LimitList {
			using Node = TypeX;
			using T = typename Either<Unit, typename Union<TypeX, LimitList<N1-1>>::T>::T;
			using D = typename Derivative<T>::T;
			static constexpr const std::size_t N = T::N;
			static constexpr const std::array<std::size_t, N + 1> A = T::A;
		};

		template <>
		struct LimitList<0> {
			using T = One;
			using D = typename Derivative<T>::T;
			static constexpr const std::size_t N = T::N;
			static constexpr const std::array<std::size_t, N + 1> A = T::A;
		};

		template <std::size_t N1>
		struct LimitTree {
			using Node = TypeX;
			using Leaf = typename LimitTree<N1 - 1>::T;
			using T = typename Either<Unit, typename Union<typename Union<TypeX, Leaf>::T, Leaf>::T>::T;
			using D = typename Derivative<T>::T;
			static constexpr const std::size_t N = T::N;
			static constexpr const std::array<std::size_t, N + 1> A = T::A;
		};

		template <>
		struct LimitTree<0> {
			using T = One;
			using D = typename Derivative<T>::T;
			static constexpr const std::size_t N = T::N;
			static constexpr const std::array<std::size_t, N + 1> A = T::A;
		};

	}

}



#endif