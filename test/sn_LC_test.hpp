#ifndef SN_TEST_LC_H
#define SN_TEST_LC_H

#include "sn_CommonHeader_test.h"

namespace sn_LC_test {
	using namespace sn_LC;
	void sn_lc_test() {
		using v_env = EnvLookup<3, Binding<2, Succ<Zero>, Binding<3, Zero, EmptyEnv>>>::result;
		int v = v_env::value;

		// ((lambda (x) x) 2)
		enum { X };
		using x_eval = Eval<Application<Lambda<X, Reference<X>>, Literal<Succ<Succ<Zero>>>>, EmptyEnv>::result;
		int x = x_eval::value;

		// (if #f 0 1)
		int y = Eval<If<Literal<False>, Literal<Zero>, Literal<Succ<Zero>>>, EmptyEnv>::result::value;
	
		using z_t = NaturalNumber<10>;
		int z = z_t::value;
	}
}











#endif