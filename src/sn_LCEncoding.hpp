#ifndef SN_LC_ENCODING_H
#define SN_LC_ENCODING_H

#include "sn_CommonHeader.h"
#include "sn_LC.hpp"

// Not common encoding. Mogensen�CScott/Church/Boehm-Berarducci
// TODO: Boehm-Berarducci Encoding ref: http://okmij.org/ftp/tagless-final/course/Boehm-Berarducci.html
namespace sn_LCEncoding {
	using namespace sn_LC;
	// datatype - foldr/accumulate - CPS
	/*
	ref: http://stackoverflow.com/questions/13575894/why-do-we-use-folds-to-encode-datatypes-as-functions
	churchfold :: (a -> b -> b) -> b -> [a] -> b
	churchfold _ z [] = z
	churchfold f z (x:xs) = f x (churchfold f z xs)
	*/
	// TODO: add real calculation operators
	// TODO: add foldr/foldl/append... ref: http://benji6.github.io/church/docs/lists.html
	
	namespace Combinator {

		enum {
			V1, V2, V3, V4,  V5, A, B, C, D, E, F
			V6, V7, V8, V9, V10, X, Y, N, M, U
		};

		/*
		
			I := λx.x
			K := λx.λy.x
			S := λx.λy.λz.x z (y z)
			B := λx.λy.λz.x (y z)
			C := λx.λy.λz.x z y
			W := λx.λy.x y y
			U := λx.λy.y (x x y)
			ω := λx.x x
			Ω := ω ω
			Y := λg.(λx.g (x x)) (λx.g (x x))
		
		*/

		// U = lambda .x lambda .y y (x x y)
		using UCombinator = Lambda<
								X, 
								Lambda<
									Y, 
									Application<
										Y, 
										Application<
											X, 
											Application<
												X, 
												Y
											>
										>
									>
								>
							>;

		// omega := lambda .x x x
		using omega = Lambda<
							V1,
							Application<
								Application<
									Reference<V1>
									Reference<V1>
								>
							>
						>;

		// Omega := omega omega
		using Omega = Application<omega, omega>;

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
	}
	
	namespace Church {
		using Nonsense = Literal<Zero>;
		enum { F, T, X, M, N, G, H, U };
		using ID = Lambda<X, Reference<X>>;
		template <typename FX, typename X>
		// lambda .f 
		//		lambda .x 
		//			x
		using ChurchZero = Application<Lambda<F, Application<ID, X>>, Nonsense>;
		// lambda .f 
		//		lambda .x 
		//			f 
		//			(lambda .f
		//				lambda .x
		//					f x ...)(f, x)
		template <std::size_t N, typename FX, typename X>
		struct ChurchNValue {
			using value = Application<
							Lambda<
								F,
								Application<
									Lambda<
										T,
										Application<
											Reference<F>,
											Reference<T>
										>
									>,
									typename ChurchNValue<N - 1, FX, X>::value
								>
							>, FX>;
		};
		// lambda .f 
		//		lambda .x 
		//			f x
		template <typename FX, typename X>
		struct ChurchNValue<1, FX, X> {
			using value = Application<
							Lambda<
								F,
								Application<
									Lambda<
										T,
										Application<
											Reference<F>,
											Reference<T>
										>
									>, 
									X
								>
							>, FX>;
		};
		template <typename FX, typename X>
		struct ChurchNValue<0, FX, X> {
			using value = Application<Lambda<F, Application<ID, X>>, Nonsense>;
		};
		template <std::size_t N, typename FX, typename X>
		using ChurchNumberT = typename ChurchNValue<N, FX, X>::value;
		
		// inc/pred/mult/sub is simple size_t operation
		template <std::size_t N, typename FX, typename X>
		struct ChurchNSuccValue {
			using value = typename ChurchNValue<N + 1, FX, X>::value;
		};
		template <std::size_t N, typename FX, typename X>
		using ChurchNSucc = typename ChurchNSuccValue<N, FX, X>::value;

		template <typename FX, typename X>
		struct ChurchTValue {
			using value = FX;
		};
		template <typename FX, typename X>
		struct ChurchFValue {
			using value = X;
		};


		enum { V1, V2, V3, V4, V5,
				V6, V7, V8, V9, V10,
		};
		// lambda .f 
		//		lambda .x 
		//			f 
		//			(lambda .f
		//				lambda .x
		//					f x ...)(f, x)
		template <std::size_t N>
		struct ChurchNV {
			using value = VarLambda<
								VarList<V1, V2>,
								Application<
									Reference<V1>,
									Application<
										typename ChurchNV<N - 1>::value,
										ValList<Reference<V1>, Reference<V2>>
									>
								>
							>;
		};
		// lambda .f 
		//		lambda .x 
		//			f x
		template <>
		struct ChurchNV<1> {
			using value = VarLambda<
								VarList<V1, V2>,
								Application<
									Reference<V1>,
									Reference<V2>
								>
							>;
		};
		template <>
		struct ChurchNV<0> {
			using value = VarLambda<
								VarList<V1, V2>,
								Reference<V2>
							>;
		};

		template <std::size_t N, typename T>
		struct ChurchNImpl {
			using value = 
				NaturalNumber<std::is_same<typename ChurchNV<N>::value, T>::value ? N : ChurchNImpl<N + 1, T>::value::value>;
		};


		template <std::size_t N>
		using ChurchNumber = typename ChurchNV<N>::value;
		// hate operations... so this is tricky and compiler-errno
		template <typename T>
		using ChurchN = typename ChurchNImpl<0, T>::value;
		
		// n f x = f^n x

		// succ = \n -> f x -> f (n f x)
		using ChurchSucc = VarLambda<
								VarList<N>,
								VarLambda<
									VarList<F, X>,
									Application<
										Reference<F>,
										Application<
											Reference<N>,
											ValList<
												Reference<F>,
												Reference<X>
											>
										>
									>
								>
							>;

		// f^(m+n) x = f^m (f^n x)
		// plus = \m n -> \f x -> m f (n f x)
		using ChurchPlus = VarLambda<
								VarList<M, N>,
								VarLambda<
									VarList<F, X>,
									Application<
										Reference<M>,
										ValList<
											Reference<F>,
											Application<
												Reference<N>,
												ValList<
													Reference<F>,
													Reference<X>
												>
											>
										>
									>
								>
							>;

		// f^(mn) x = (f^n)^m x
		// m (n f) x
		// mult = \m n -> \f x -> m (n f) x (\m n -> \f x -> m (\y -> n f y) x)
		using ChurchMult = VarLambda<
								VarList<M, N>,
								VarLambda<
									VarList<F, X>
									Application<
										Reference<M>,
										ValList<
											VarLambda<
												VarList<V1>,
												Application<
													Reference<N>,
													ValList<
														Reference<F>,
														Reference<V1>
													>
												>
											>,
											Reference<X>
										>
									>
								>
							>;

		// n m x = m^n x = f^(m^n) x
		// \m n -> \f x -> (\f' -> m n f') f x
		using ChurchExp = VarLambda<
								VarList<M, N>,
								VarLambda<
									VarList<F, X>,
									Application<
										Application<
											VarLambda<
												VarList<V1>,
												Application<
													Reference<M>,
													ValList<
														Reference<N>,
														Reference<V1>
													>
												>
											>,
											ValList<
												Reference<F>
											>
										>,
										Reference<X>
									>
								>
							>;

		// pred = \n -> \f x -> n (\g h -> h (g f)) (\u -> x) (\u -> u)
		
		// x is predefined or curried
		// value = \v -> \h -> h v
		// inc = \g h -> h (g f)
		// init = \h h x
		// const = \u x

		// init = value v
		// inc init = value (f v)
		// n inc init = value (f^n x) = value (n f x)
		// inc (value v) = value (f v)
		// extract (value v) = v
		// samesum = \n -> \f x extract (n inc init)
		// 				= \n -> \f x extract (value (n f x))
		//					= \n -> \f x -> n f x
		//						= \n n

		// inc const = value (f x)
		// inc (inc const) = value (f (f x))
		// n inc const = value (f^(n-1) x) = value ((n-1) f x)
		// pred    = \n -> \f x extract (n inc const)
		//				= \n -> \f x extract value ((n-1) f x)
		//					= \n -> \f x -> (n-1) f x
		//						= \n n-1

		// template parameter is free variables
		template <typename T>
		using ChurchPredConst = VarLambda<
									VarList<X>,
									T
								>;

		template <typename F>
		using ChurchPredInc = VarLambda<
									VarList<G, H>,
									Application<
										Reference<H>,
										Application<
											Reference<G>,
											F
										>
									>
								>;

		using ChurchPredInit = VarLambda<
									VarList<V1>,
									Reference<V1>
								>;

		// Just pretend we have curry implementation
		using ChurchPred = VarLambda<
								VarList<N>,
								VarLambda<
									VarList<F, X>,
									Application<
										Application<
											Reference<N>,
											ValList<
												ChurchPredInc<Reference<F>>,
												ChurchPredConst<Reference<X>>
											>
										>,
										ChurchPredInit
									>
								>
							>;

		// m - n
		// minus = \m n -> \f x -> ((n pred) m) f x
		using ChurchMinus = VarLambda<
								VarList<M, N>,
								VarLambda<
									VarList<F, X>,
									Application<
										Application<
											Reference<N>,
											ValList<
												ChurchPred,
												Reference<M>
											>
										>
										ValList<
											Reference<F>,
											Reference<X>
										>
									>
								>
							>;
										

		using ChurchTrue = VarLambda<
								VarList<V1, V2>, 
								Reference<V1>
							>;
		using ChurchFalse = VarLambda<
								VarList<V1, V2>,
								Reference<V2>
							>;
		using ChurchAnd = VarLambda<
								VarList<V1, V2>,
								Application<
									Reference<V1>,
									ValList<Reference<V2>, Reference<V1>>
								>
							>;
		using ChurchOr = VarLambda<
								VarList<V1, V2>,
								Application<
									Reference<V1>,
									ValList<Reference<V1>, Reference<V2>>
								>
							>;
		using ChurchNot = VarLambda<
								VarList<V1>,
								Application<
									Reference<V1>,
									ValList<ChurchFalse, ChurchTrue>
								>
							>;
		using ChurchXor = VarLambda<
								VarList<V1, V2>, 
								Application<
									Reference<V1>,
									ValList<
										Application<
											ChurchNot,
											Reference<V2>
										>,
										Reference<V2>
									>
								>
							>;
		using ChurchIf = VarLambda<
							VarList<V1, V2, V3>,
							Application<
								Reference<V1>,
								ValList<
									Reference<V2>,
									Reference<V3>
								>
							>
						>;
		using ChurchIsZero = VarLambda<
								VarList<V1>,
								Application<
									Reference<V1>,
									ValList<
										Lambda<V2, ChurchFalse>,
										ChurchTrue
									>
								>
							>;
		using ChurchPair = VarLambda<
								VarList<V1, V2>,
								VarLambda<
									VarList<V3>,
									Application<
										Reference<V3>,
										ValList<
											Reference<V1>,
											Reference<V2>
										>
									>
								>
							>;
		using ChurchFirst = VarLambda<
								VarList<V1>,
								Application<
									Reference<V1>,
									ValList<ChurchTrue>
								>
							>;
		using ChurchSecond = VarLambda<
									VarList<V1>,
									Application<
										Reference<V1>,
										ChurchFalse
									>
							  >;
		/*
		cons = pair
		head = first
		tail = second
		nil  = false
		isnil= lambda .l l(lambda .h .t .d false)(true)
		--------or--------
		nil  = pair true true
		isnil= first
		cons = lambda .h lambda .t (pair (false pair (h, t)))
		head = lambda .z (first (second z))
		tail = lambda .z (second (second z))
		--------or-------- foldr version
		nil  = lambda .c lambda .n n
		isnil= lambda .l l(lambda .h lambda .t false) true
		cons = lambda .h lambda .t lambda .c lambda.n c h (t c n)
		head = lambda .l l(lambda .h lambda .t h) false
		tall = lambda .l lambda .c lambda .n l(lambda ,h lambda .t lambda .g g h (t c)) (lambda .t n) false
		*/
								
	}

	// datatype - case/ADT/parsec - CPS
	namespace Scott {
		enum {
			V1, V2, V3, V4,  V5, A, B, C,
			V6, V7, V8, V9, V10, X, Y, N
		};
		// This should be extended to non-determined varadic varlist
		using ScottZero = VarLambda<
								VarList<X, Y>,
								Reference<X>
							>;
		using ScottSucc = VarLambda<
								VarList<N, X, Y>,
								Application<
									Reference<Y>,
									Reference<N>
								>
							>;
		// case(n)(a)(f) -> a (if n == 0) | f (if n == succ(....(0))
		using ScottCase = VarLambda<
								VarList<N, X, Y>,
								Application<
									Reference<N>,
									ValList<
										Reference<X>,
										Reference<Y>
									>
								>
							>;
		// Mogensen-Scott Encoding
		// Constructor A - a variable (arity 1, not recursive)  [Nil]
		// Constructor B - function application (arity 2, recursive in both arguments)  [Cons]
		// Constructor C - lambda-abstraction (arity 1, recursive).   [Apply-Func]
		// Example: ../snippets/sc_scott_pattern_match.cpp
		template <typename T>
		struct MSET {};

		// A(x)
		template <std::size_t I>
		struct MSET<Reference<I>> {
			using result = VarLambda<
								VarList<A, B, C>,
								Application<
									Reference<A>,
									Reference<I>
								>
							>;
		};
		// A(1)
		template <typename T>
		struct MSET<Literal<T>> {
			using result = VarLambda<
								VarList<A, B, C>,
									Application<
										Reference<A>,
										Literal<T>
									>
							>;
		};

		// B(x, xs)
		template <typename Arg, typename ...Args>
		struct MSET<ValList<Arg, Args...>> {
			using result = VarLambda<
								VarList<A, B, C>,
								Application<
									Reference<B>,
									ValList<Arg, ValList<Args...>>
								>
							>;
		};

		// B(C(F), xs)
		template <typename F, typename ...Args>
		struct MSET<Application<F, ValList<Args...>>> {
			using result = VarLambda<
								VarList<A, B, C>,
									Application<
										Reference<B>,
										ValList<
											typename MSET<F>::result, 
											ValList<Args...>
										>
								>
							>;
		};

		// C(F)
		template <typename V, typename T>
		struct MSET<VarLambda<V, T>> {
			using result = VarLambda<
								VarList<A, B, C>,
								Application<
									Reference<C>,
									VarLambda<
										V, 
										typename MSET<T>::result
									>
								>
							>;
		};

		template <typename T>
		using MSE = typename MSET<T>::result;
	}

	// ref: https://hackage.haskell.org/package/algebraic-graphs-0.0.4/docs/Algebra-Graph-Fold.html
	// Boehm interpretation: 
	//		f in ChurchNumber -> view type deal with expression type
	//		ChurchNumber -> view'(view' (.... (view' exp)))
	// exp   : datatype (encoded: with attribute f1, ..., fn) 
	//				in lambda-calculus, exp: exp_f1 | ... | exp_fn
	// expc  : lambda .a lambda .b .... [f1 -> f2 -> ... -> fn -> exp] (Case | Scott Encoding Constructor)
	// exp_fi: [f1 -> ... -> fn -> exp -> exp -> ... -> exp] ::= lambda (.f1 ... .fn .exp...) fi(f1, ..., fn, exp...) | Or exp->fi(exp...)
	// exp_fj: lit [f1 -> ... -> fn -> T -> exp] ::= lambda(.f1 ... .fn .T) fj(f1, ..., fn, T)                        | Or exp->fj(T)
	// view  : lambda .exp [f1 -> ... -> fn -> exp -> V] exp(f1, ... fn) (Foldr | Church Encoding)
	// exp_f : lit, neg, add
	// f     : dlit, dneg, dadd
	// view (add (lit 8) (add (lit 1) (lit 2)))
	namespace Boehm {

	}

	// TODO: add Church number
	// ref: https://wiki.haskell.org/Type_SK
	// ref: https://weblogs.asp.net/dixin/lambda-calculus-via-c-sharp-21-ski-combinator-calculus
	namespace SKI {
		enum {
			V1, V2, V3, V4, V5, A, B, C,
			V6, V7, V8, V9, V10, X, Y, Z
		};
		// I : \x -> x
		using I = VarLambda<VarList<X>, Reference<X>>;
		
		// K : \x, y -> x  (AK: A -> (B -> A))
		using K = VarLambda<VarList<X, Y>, Reference<X>>;
		
		// S : \x, y, z -> x(z, y(z))  (AS: A -> (B -> C) -> (A -> B) -> (A -> C))
		using S = VarLambda<
						VarList<X, Y, Z>,
						Application<
							Reference<X>,
							ValList<
								Reference<Z>,
								Application<
									Reference<Y>,
									Reference<Z>
								>
							>
						>
					>;

		// lota = \x -> x(S, K)
		using lota = VarLambda<
							VarList<X>,
							Application<
								Reference<X>,
								ValList<S, K>
							>
						>;

		// SII = \x -> I(x)(I(x)) -> x(x)
		using SII = VarLambda<
							VarList<Y>,
							Application<
								Application<
									I,
									ValList<Reference<Y>>
								>,
								ValList<
									Application<
										I,
										ValList<Reference<Y>>
									>
								>
							>
						>;
		
		// Reverse: \x \y: S(K(S, I))(K(x, y))
		using Reverse = VarLambda<
							VarList<V1, V2>,
							Application<
								Application<
									S,
									ValList<
										Application<
											K,
											ValList<S, I>
										>,
										K,
										Reference<V1>
									>
								>,
								ValList<
									Reference<V2>
								>
							>
						>;

		using T = K;
		using F = VarLambda<
						VarList<V1, V2>,
						Application<
							Application<
								K,
								ValList<
									I,
									Reference<V1>
								>
							>,
							ValList<Reference<V2>>
						>
					>;
						
		using Not = VarLambda<
						VarList<V1>,
						Application<
							F,
							ValList<T, Reference<V1>>
						>
					>;
	}

	namespace BCKW {
		enum {
			V1, V2, V3, V4, V5,
			V6, V7, V8, V9, V10, X, Y, Z
		};

		// B ::= x o y
		using B = VarLambda<
						VarList<X, Y, Z>,
						Application<
							Reference<X>,
							ValList<
								Application<
									Reference<Y>,
									Reference<Z>
								>
							>
						>
					>;

		// C ::= x y z -> x z y
		using C = VarLambda<
						VarList<X, Y, Z>,
						Application<
							Reference<X>,
							ValList<
								Reference<Z>,
								Reference<Y>
							>
						>
					>;

		// K ::= x y -> x
		using K = VarLambda<VarList<X, Y>, Reference<X>>;

		// W ::= x y -> x y y
		using W = VarLambda<
						VarList<X, Y>,
						Application<
							Reference<X>,
							ValList<
								Reference<Y>,
								Reference<Y>
							>
						>
					>;

		/*
		B = S (K S) K
		C = S (S (K (S (K S) K)) S)(K K)
		K = K
		W = S S (S K)
		
		I = W K
		K = K
		S = B (B (B W) C) (B B)[1] = B (B B B W B) C
		*/
	}
}







#endif