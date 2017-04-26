#include <array>
#include <utility>

namespace logseq {

	// alias for std::integer_sequence, for brevity
	template <typename T, T... Is>
	using intseq_t = std::integer_sequence<T, Is...>;

	template <typename A, typename B>
	struct concat;

	// A: intseq_t
	// B: intseq_t
	// return: typename intseq_t
	//
	// Example:
	//   concat_t< intseq_t<int,0,1,2>, intseq_t<int,0,1,2,3> >
	//     => intseq_t<int,0,1,2,3,4,5,6>
	template <typename A, typename B>
	using concat_t = typename concat<A,B>::type;

	template <typename T, T... As, T... Bs>
	struct concat<intseq_t<T, As...>, intseq_t<T, Bs...>> {
	    using type = intseq_t<T, As..., (sizeof...(As) + Bs)...>;
	};

	template <typename T, T N, typename = void>
	struct logseq;

	// T: integer type
	// N: number of elements
	// return: typename std::make_integer_sequence<T,N>
	// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66059
	template <typename T, T N>
	using logseq_t = typename logseq<T,N>::type;

	template <typename T, T N, typename>
	struct logseq {
	    using type = concat_t<logseq_t<T,N/2>,logseq_t<T,N-N/2>>;
	};

	template <typename T, T N>
	struct logseq<T,N,typename std::enable_if<N==0>::type> {
	    using type = intseq_t<T>;
	};

	template <typename T, T N>
	struct logseq<T,N,typename std::enable_if<N==1>::type> {
	    using type = intseq_t<int,0>;
	};

} // namespace logseq


constexpr bool isPrimeLoop(int i, int k) {
    return (k*k > i)? true : (i%k == 0) ? false : isPrimeLoop(i, k + 1);
}

constexpr bool isPrime(int i) {
    return isPrimeLoop(i, 2);
}

constexpr int nextPrime(int k) {
    return isPrime(k)?k:nextPrime(k + 1);
}

constexpr int getPrimeLoop(int i, int k) {
    return (i == 0)?k:
        (i % 2)?getPrimeLoop(i-1, nextPrime(k + 1)):
        getPrimeLoop(i/2, getPrimeLoop(i/2, k));
}

constexpr int getPrime(int i) {
    return getPrimeLoop(i, 2);
}

template <typename T, T... Is>
constexpr auto gen_primes_helper(std::integer_sequence<T, Is...>) {
    return std::array<T, sizeof...(Is)>{{getPrime(Is)...}};
}

// T: integer type
// N: number of elements
// return: std::array<T,N>
template <typename T, T N>
constexpr auto gen_primes() {
    using logseq::logseq_t;
    return gen_primes_helper(logseq_t<T,N>());
}

constexpr auto primes = gen_primes<int,1024>();