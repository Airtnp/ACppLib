#ifndef SN_LC_ENCODING_H
#define SN_LC_ENCODING_H

#include "sn_CommonHeader.h"
#include "sn_LC.hpp"

// Not common encoding. Mogensen¨CScott/Church/Boehm-Berarducci
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
	namespace Church {
		using Nonsense = Literal<Zero>;
		enum { F, T };
		using ID = Lambda<T, Reference<T>>;
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
		struct ChurchSuccValue {
			using value = typename ChurchNValue<N + 1, FX, X>::value;
		};
		template <std::size_t N, typename FX, typename X>
		using ChurchSucc = typename ChurchSuccValue<N, FX, X>::value;

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

}







#endif