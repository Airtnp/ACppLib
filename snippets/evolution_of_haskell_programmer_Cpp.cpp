#include <cstddef>
#include <tuple>
#include <utility>
#include <initializer_list>
#include <iostream>

// ref: http://www.willamette.edu/~fruehr/haskell/evolution.html
// TODO: complete the left...
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

	template <std::size_t N>
	struct make_reverse_index_sequence_impl
		: concate<
		typename make_reverse_index_sequence_impl<N - N / 2>::type,
		typename make_reverse_index_sequence_impl<N / 2>::type
		> {};

	template <>
	struct make_index_sequence_impl<0> : index_sequence<> {};
	template <>
	struct make_index_sequence_impl<1> : index_sequence<1> {};

	template <>
	struct make_reverse_index_sequence_impl<0> : index_sequence<> {};
	template <>
	struct make_reverse_index_sequence_impl<1> : index_sequence<1> {};


	template <std::size_t N>
	using make_positive_index_sequence = typename make_index_sequence_impl<N>::type;
	template <std::size_t N>
	using make_reverse_positive_index_sequence = typename make_reverse_index_sequence_impl<N>::type;
}
using sequence::index_sequence;
using sequence::make_positive_index_sequence;
using sequence::make_reverse_positive_index_sequence;


/*
fac n = if n == 0
		then 1
		else n * fac (n-1)
*/
int fac1(int n) {
	if (n == 0)
		return 1;
	else
		return n * fac1(n - 1);
}

/*
fac = (\(n) ->
		(if ((==) n 0)
			then 1
			else ((*) n (fac ((-) n 1)))))
*/
int fac2(int n) {
	return [](int n) -> int {
		if (n == 1)
			return 1;
		else
			return n * fac2(n - 1);
	}(n);
}

/*
fac  0    =  1
fac (n+1) = (n+1) * fac n
*/
template <std::size_t I>
constexpr int fac3() {
	if constexpr (I == 0)
		return 1;
	else
		return I * fac3<I - 1>();
}

/*
fac n = foldr (*) 1 [1..n]
fac n = foldl (*) 1 [1..n]
*/

template <std::size_t I>
constexpr int fac4_impl(std::size_t n, index_sequence<I>) {
	return n * I;
}

template <std::size_t I, std::size_t ...Is>
constexpr int fac4_impl(std::size_t n, index_sequence<I, Is...>) {
	return fac4_impl(n * I, index_sequence<Is...>{});
}


template <std::size_t N>
constexpr int fac4_r() {
	return fac4_impl(N, make_reverse_positive_index_sequence<N>{});
}

template <std::size_t N>
constexpr int fac4_l() {
	return fac4_impl(N, make_positive_index_sequence<N>{});
}

/*
foldr -> foldl
fac n = foldr (\x g n -> g (x*n)) id [1..n] 1
*/



int main() {
	std::cout << fac4_r<10>();
}