#ifndef SN_LC_H
#define SN_LC_H

#include "sn_CommonHeader.h"

// Lambda-calculus
// ref: http://matt.might.net/articles/c++-template-meta-programming-with-lambda-calculus/
// ref: https://github.com/Cheukyin/TemplatedPL
// TODO: look at MK/lambda-calculus
// TODO: add more combinators
// TODO: add call/cc (implement eval/apply/if/... in CPS style with continuation)
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

#ifndef _MSC_VER
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
#endif
	/* Laji MSVC, failed in this deduction
 	using T = MatchNumber<0, Zero>;
	static_assert(std::is_same<T, Literal<True>>::value, "MatchNumber Failed");
	*/

	// Add multi-var lambda
	// Cpp doesn't support non-type | type parameter
	template <std::size_t ...I>
	struct VarList;

	template <typename ...Args>
	struct ValList;

	template <typename Val, typename Body>
	struct VarLambda;

	template <typename Var, typename Val, typename Env>
	struct VarBinding;

	template <std::size_t I1, std::size_t ...I, typename Arg, typename ...Args, typename Env>
	struct VarBinding<VarList<I1, I...>, ValList<Arg, Args...>, Env> {
		using result = Binding<I1, Arg, typename VarBinding<VarList<I...>, ValList<Args...>, Env>::result>;
	};

	template <typename Env>
	struct VarBinding<VarList<>, ValList<>, Env> {
		using result = Env;
	};

	template <std::size_t ...I, typename Body, typename Env>
	struct Eval<VarLambda<VarList<I...>, Body>, Env> {
		using result = Closure<VarLambda<VarList<I...>, Body>, Env>;
	};

	template <typename F, typename ...Args, typename Env>
	struct Eval<Application<F, ValList<Args...>>, Env> {
		using result = typename Apply<typename Eval<F, Env>::result, typename Eval<ValList<Args...>, Env>::result>::result;
	};

	template <typename ...Args, typename Env>
	struct Eval<ValList<Args...>, Env> {
		using result = ValList<typename Eval<Args, Env>::result...>;
	};

	template <std::size_t ...I, typename Body, typename Env, typename ...Args>
	struct Apply<Closure<VarLambda<VarList<I...>, Body>, Env>, ValList<Args...>> {
		using result = typename Eval<Body, typename VarBinding<VarList<I...>, ValList<Args...>, Env>::result>::result;
	};

	// support operation
	template <typename T, typename U>
	struct Add;
	template <typename T, typename U>
	struct Sub;
	template <typename T, typename U>
	struct Mul;
	template <typename T, typename U>
	struct Div;
	template <typename T, typename U>
	struct Mod;
	
	template <typename T, typename U, typename Env>
	struct Eval<Add<T, U>, Env> {
		using result = typename NaturalNumber<Eval<T, Env>::result::value + Eval<U, Env>::result::value>::type;
	};

	template <typename T, typename U, typename Env>
	struct Eval<Sub<T, U>, Env> {
		using result = typename NaturalNumber<Eval<T, Env>::result::value - Eval<U, Env>::result::value>::type;
	};

	template <typename T, typename U, typename Env>
	struct Eval<Mul<T, U>, Env> {
		using result = typename NaturalNumber<Eval<T, Env>::result::value * Eval<U, Env>::result::value>::type;
	};

	template <typename T, typename U, typename Env>
	struct Eval<Div<T, U>, Env> {
		using result = typename NaturalNumber<Eval<T, Env>::result::value / Eval<U, Env>::result::value>::type;
	};

	template <typename T, typename U, typename Env>
	struct Eval<Mod<T, U>, Env> {
		using result = typename NaturalNumber<Eval<T, Env>::result::value % Eval<U, Env>::result::value>::type;
	};

	template <typename T, typename U>
	struct Equal;

	template <typename T, typename U>
	struct UnEqual;


	template <bool B>
	struct Boolean {
		using result = False;
	};

	template <>
	struct Boolean<true> {
		using result = True;
	};

	template <typename T, typename U, typename Env>
	struct Eval<Equal<T, U>, Env> {
		using result = typename Boolean<Eval<T, Env>::result::value == Eval<U, Env>::result::value>::result;
	};

	template <typename T, typename U, typename Env>
	struct Eval<UnEqual<T, U>, Env> {
		using result = typename Boolean<Eval<T, Env>::result::value != Eval<U, Env>::result::value>::result;
	};

	// support map/filter/reduce/sum/append

	template <typename L1, typename L2>
	struct ValListAppend;

	template <typename ...T, typename ST>
	struct ValListAppend<ValList<T...>, ST> {
		using type = ValList<T..., ST>;
	};

	template <typename ...T1, typename ...T2>
	struct ValListAppend<ValList<T1...>, ValList<T2...>> {
		using type = ValList<T1..., T2...>;
	};

	template <typename L, typename V>
	struct Map;

	template <typename L, typename ...Args>
	struct Map<L, ValList<Args...>>;

	template <typename L, typename ...Args, typename Env>
	struct Eval<Map<L, ValList<Args...>>, Env> {
		using result = ValList<typename Eval<Application<L, Args>, Env>::result...>;
	};

	template <typename V>
	struct Sum;

	template <typename ...Args>
	struct Sum<ValList<Args...>>;

	template <typename Arg, typename ...Args, typename Env>
	struct Eval<Sum<ValList<Arg, Args...>>, Env> {
		using result = typename NaturalNumber<Eval<Arg, Env>::result::value + Eval<Sum<ValList<Args...>>, Env>::result::value>::type;
	};

	template <typename Env>
	struct Eval<Sum<ValList<>>, Env> {
		using result = Zero;
	};

	template <typename L, typename V>
	struct Reduce;

	template <typename L, typename ...Args>
	struct Reduce<L, ValList<Args...>>;

	template <typename L, typename Arg1, typename Arg2, typename ...Args, typename Env>
	struct Eval<Reduce<L, ValList<Arg1, Arg2, Args...>>, Env> {
		using first = typename Eval<Application<Application<L, Arg1>, Arg2>, Env>::result;
		using result = typename Eval<Reduce<L, ValList<first, Args...>>, Env>::result;
	};

	template <typename L, typename Arg, typename Env>
	struct Eval<Reduce<L, ValList<Arg>>, Env> {
		using result = Arg;
	};

	template <typename L, typename V>
	struct Filter;

	template <typename L, typename ...Args>
	struct Filter<L, ValList<Args...>>;

	template <typename Cond, typename Arg>
	struct FilterImpl {
		using result = ValList<Arg>;
	};

	template <typename Arg>
	struct FilterImpl<False, Arg> {
		using result = ValList<>;
	};

	template <typename L, typename Arg, typename ...Args, typename Env>
	struct Eval<Filter<L, ValList<Arg, Args...>>, Env> {
		using Cond = typename Eval<Equal<typename Eval<Arg, Env>::result, True>, Env>::result;
		using result = typename ValListAppend<typename FilterImpl<Cond, Arg>::result, typename Eval<Filter<L, ValList<Args...>>, Env>::result>::type;
	};

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

	template <std::size_t X, std::size_t Y>
	using TCombinator = Application<
							Lambda<X,
								Lambda<Y, 
									Application<Reference<Y>, 
										Application<Reference<X>, 
											Application<Reference<X>, 
												Reference<Y>
											>
										>
									>
								>
							>, 
							Lambda<X,
								Lambda<Y, 
									Application<Reference<Y>, 
										Application<Reference<X>, 
											Application<Reference<X>, 
												Reference<Y>
											>
										>
									>
								>
							>
						>;

	template <std::size_t X, std::size_t Y, typename L>
	using TFunc = Application<TCombinator<X, Y>, L>;

	// TODO: Add call/cc
	// To implement this, we need full-context-CPS / Stimulaton Device
	// Eval<F, Env, Cont>
	// If -> Branch<Cont1, Cont2, Env, Cont>
	// ref: https://github.com/Cheukyin/TemplatedPL/blob/master/evaluator.hpp

}









#endif