#ifndef SN_TEST_LC_H
#define SN_TEST_LC_H

#include "sn_CommonHeader_test.h"

namespace sn_LC_test {
	using namespace sn_LC;
	void sn_lc_test() {
		// 2 => 0, 3 => 0
		using v_env = EnvLookup<3, Binding<2, Succ<Zero>, Binding<3, Zero, EmptyEnv>>>::result;
		int v = v_env::value;

		// ((lambda .x x) 0)
		enum { X };
		using x_eval = Eval<Application<Lambda<X, Reference<X>>, Literal<Zero>>, EmptyEnv>::result;
		int x = x_eval::value;

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
		int x = t_eval::value;

		// (if #f 0 1)
		int y = Eval<If<Literal<False>, Literal<Zero>, Literal<Succ<Zero>>>, EmptyEnv>::result::value;
	
		using z_t = NaturalNumber<10>;
		int z = z_t::value;
	}
}











#endif