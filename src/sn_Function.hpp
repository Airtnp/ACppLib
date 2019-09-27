#ifndef SN_FUNCTION_H
#define SN_FUNCTION_H

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"
#include "sn_Type.hpp"

#include "sn_Function/curry.hpp"
#include "sn_Function/failable.hpp"
#include "sn_Function/func.hpp"
#include "sn_Function/lens.hpp"
#include "sn_Function/memoize.hpp"
#include "sn_Function/operation.hpp"
#include "sn_Function/pipeline.hpp"
#include "sn_Function/trampoline.hpp"
#include "sn_Function/type_curry.hpp"
#include "sn_Function/Y.hpp"

namespace sn_Function {
	
	using function::make_func;
	using currying::make_curry;
	using currying::make_single_curry;
	using currying::make_multi_curry;
	using currying::make_curry_proxy;
	using lambda_currying::make_lambda_curry;
	using combining::make_combine;
	using combining::make_homomorphy_combine;
	using maybe_just::maybe;
	using maybe_just::just;
	using lazy::make_lazy;
	using functor_wrapper::make_functor_wrapper;
	using trampoline::make_trampoline;
	using trampoline::make_trampoline_wrapper;
	using YCombinator::Y;
	using YCombinator::YA;
	// using template_currying::TPC;

}





#endif