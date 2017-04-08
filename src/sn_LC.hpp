#ifndef SN_LC_H
#define SN_LC_H

#include "sn_CommonHeader.h"

// Lambda-calculus
// ref: http://matt.might.net/articles/c++-template-meta-programming-with-lambda-calculus/
// TODO: look at MK/lambda-calculus
// TODO: add more combinators
// TODO: add compatitble operator (+, -, * /)
// TODO: support tuple/list (lambda var1, var, ....)
namespace sn_LC {
	struct Zero {
		enum { value = 0, };
	};

	template <typename N>
	struct Succ {
		enum { value = N::value + 1 };
	};

	template <std::size_t N>
	struct NaturalNumber {
		using type = Succ<typename NaturalNumber<N-1>::type>;
		enum { value = type::value };
	};

	template <>
	struct NaturalNumber<0> {
		using type = Zero;
		enum { value = Zero::value };
	};


	template <std::size_t FormalName, typename Body>
	struct Lambda {};

	template <typename F, typename Arg>
	struct Application {};

	template <std::size_t Name>
	struct Reference {};

	template <typename Cond, typename Then, typename Else>
	struct If {};

	template <typename T>
	struct Literal {};

	struct EmptyEnv {};

	template <std::size_t Name, typename Value, typename Env>
	struct Binding {};

	template <std::size_t Name, typename Env>
	struct EnvLookup {};

	template <std::size_t Name>
	struct EnvLookup<Name, EmptyEnv> {};

	template <std::size_t Name, typename Value, typename Env>
	struct EnvLookup<Name, Binding<Name, Value, Env>> {
		using result = Value;
	};

	// Deal with recursively binding
	template <std::size_t Name1, std::size_t Name2, typename Value2, typename Env>
	struct EnvLookup<Name1, Binding<Name2, Value2, Env>> {
		using result = typename EnvLookup<Name1, Env>::result;
	};

	template <typename Lambda, typename Env>
	struct Closure {};

	struct True {};
	struct False {};

	template <typename Exp, typename Env>
	struct Eval {};

	template <typename Proc, typename Value>
	struct Apply {};

	template <typename T, typename Env>
	struct Eval<Literal<T>, Env> {
		using result = T;
	};

	template <std::size_t Name, typename Env>
	struct Eval<Reference<Name>, Env> {
		using result = typename EnvLookup<Name, Env>::result;
	};

	template <std::size_t Name, typename Body, typename Env>
	struct Eval<Lambda<Name, Body>, Env> {
		using result = Closure<Lambda<Name, Body>, Env>;
	};

	template <typename F, typename Arg, typename Env>
	struct Eval<Application<F, Arg>, Env> {
		using result = typename Apply<typename Eval<F, Env>::result, typename Eval<Arg, Env>::result>::result;
	};

	template <typename Then, typename Else, typename Env>
	struct Eval<If<True, Then, Else>, Env> {
		using result = typename Eval<Then, Env>::result;
	};

	template <typename Then, typename Else, typename Env>
	struct Eval<If<False, Then, Else>, Env> {
		using result = typename Eval<Else, Env>::result;
	};

	template <typename Cond, typename Then, typename Else, typename Env>
	struct Eval<If<Cond, Then, Else>, Env> {
		using result = typename Eval<If<typename Eval<Cond, Env>::result, Then, Else>, Env>::result;
	};

	template <std::size_t Name, typename Body, typename Env, typename Value>
	struct Apply<Closure<Lambda<Name, Body>, Env>, Value> {
		using result = typename Eval<Body, Binding<Name, Value, Env>>::result;
	};

	template <std::size_t N, typename NN>
	struct MatchNumberT {
		using T = Literal<False>;
	};

	template <typename NN>
	struct MatchNumberT<Eval<Literal<NN>, EmptyEnv>::result::value, NN> {
		using T = Literal<True>;
	};
	
	template <std::size_t N, typename NN>
	using MatchNumber = typename MatchNumberT<N, NN>::T;

	/* Laji MSVC, failed in this deduction
 	using T = MatchNumber<0, Zero>;
	static_assert(std::is_same<T, Literal<True>>::value, "MatchNumber Failed");
	*/

	// Y :   lambda .f ( lambda .x (f(x)(x)) lambda .x f(x)(x) ) or lambda .f (lambda .u u(u))(lambda.x f(x)(x))
	// ref: http://picasso250.github.io/2015/03/31/reinvent-y.html
	template <std::size_t F, std::size_t U, std::size_t X>
	using YCombinator = Lambda<F, 
							Application<
								Lambda<U, 
									Application<Reference<U>,
										Reference<U>
									>
								>, 
								Lambda<X,
									Application<
										Application<Reference<F>, 
											Reference<X>
										>, Reference<X>
									>
								>
							>
						>;

	template <std::size_t F, std::size_t U, std::size_t X, typename L>
	using YFunc = Application<YCombinator<F, U, X>, L>;


}









#endif