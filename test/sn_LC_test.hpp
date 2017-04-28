#ifndef SN_TEST_LC_H
#define SN_TEST_LC_H

#include "sn_CommonHeader_test.h"

namespace sn_LC_test {
	using namespace sn_LC;
	using namespace sn_LCEncoding::Church;
	void sn_lc_test() {
		// 2 => 0, 3 => 0
		using v_env = EnvLookup<3, Binding<2, Succ<Zero>, Binding<3, Zero, EmptyEnv>>>::result;
		int v = v_env::value;

		// ((lambda .x x) 0)
		enum { P };
		using x_eval = Eval<Application<Lambda<P, Reference<P>>, Literal<Zero>>, EmptyEnv>::result;
		int x = x_eval::value;

		// lambda .f (lambda .x x)
		enum { FX, S };
		using ID = Lambda<FX, Reference<FX>>;
		using ChurchZero = Lambda<S, Application<ID, Literal<Zero>>>;
		constexpr int x1 = Eval<Application<ChurchZero, Literal<Succ<Zero>>>, EmptyEnv>::result::value;

		using Square = Lambda<P, Mul<Reference<P>, Reference<P>>>;
		using PSquare = Application<ChurchNumber<2>, ValList<Square, Literal<NaturalNumber<2>>>>;
		constexpr int x2 = Eval<PSquare, EmptyEnv>::result::value;

		// Not true -> false 0 2 -> C2 ^2 2 -> 16
		using NotTrue = Application<ChurchNot, ValList<ChurchTrue>>;
		using FalseEval = Application<NotTrue, ValList<ChurchNumber<0>, ChurchNumber<2>>>;
		using TestFalse = Application<FalseEval, ValList<Square, Literal<NaturalNumber<2>>>>;
		constexpr int x3 = Eval<TestFalse, EmptyEnv>::result::value;

		// first (pair C0 C1) -> C0 ^2 2 -> 2
		using PairP = Application<ChurchPair, ValList<ChurchNumber<0>, ChurchNumber<1>>>;
		using FirstP = Application<ChurchFirst, ValList<PairP>>;
		using TestP = Application<FirstP, ValList<Square, Literal<NaturalNumber<2>>>>;
		constexpr int x4 = Eval<TestP, EmptyEnv>::result::value;


		/* (((lambda .f 
				lambda .x f(x)
			 ) 
			 (
				lambda .x x
			 )    // g(f) => f(x), f(x) => x g(f) => x
			) 2)
		*/
		enum { X, T, F };
		using x_func = Lambda<X, Reference<X>>;
		using t_wrapper = Lambda<F,
							Lambda<T, Application<Reference<F>, Reference<T>>>
							>;
		using t_func = Application<t_wrapper, x_func>;
		using t_eval = Eval<Application<t_func, Literal<Succ<Zero>>>, EmptyEnv>::result;
		int x5 = t_eval::value;

		// (if #f 0 1)
		int y = Eval<If<Literal<False>, Literal<Zero>, Literal<Succ<Zero>>>, EmptyEnv>::result::value;
	
		using z_t = NaturalNumber<10>;
		int z = z_t::value;

		// (lambda .x .y y)(2, 1)
		enum { T1, T2 };
		using v_func = Eval<Application<
								VarLambda<
									VarList<T1, T2>, Reference<T2>
								>, ValList<Literal<Zero>, Literal<Succ<Zero>>>
							>, EmptyEnv>::result;
		constexpr int w = v_func::value;
		
		enum { P2, P3 };
		using SKIF = sn_LCEncoding::SKI::F;
		using SKIT = sn_LCEncoding::SKI::T;
		using SKIK = sn_LCEncoding::SKI::K;
		using SKII = sn_LCEncoding::SKI::I;
		constexpr int pw = Eval<VarApplication<
									Curry<SKIK, ValList<SKII>>, 
									ValList<Literal<Zero>, Literal<Succ<Succ<Zero>>>>
								>, EmptyEnv>::result::value;
	}

}











#endif