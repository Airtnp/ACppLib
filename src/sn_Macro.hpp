#ifndef SN_MACRO_H
#define SN_MACRO_H

#include "sn_CommonHeader.h"

namespace sn_Macro { //useless namespace

	//solve VS2015 macro expansion bugs

#define MACRO_EXPAND(...) __VA_ARGS__
#define APPLY_VARIADIC_MACRO(macro, ...) MACRO_EXPAND(macro(__VA_ARGS__))

#define MACRO_CONCAT(A, B) MACRO_CONCAT_IMPL(A, B)
#define MACRO_CONCAT_IMPL(A, B) A##_##B

#define MACRO_RAW_CONCAT(A, B) MACRO_RAW_CONCAT_IMPL(A, B)
#define MACRO_RAW_CONCAT_IMPL(A, B) A##B

	// static char THIS_FILE[] = __FILE__
#define DEBUG_NEW(THIS_FILE, __FILE__)

}

#endif