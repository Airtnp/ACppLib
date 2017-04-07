#ifndef SN_TEST_H
#define SN_TEST_H

#include "sn_CommonHeader.h"
#include "sn_Log.hpp"
#include "sn_StdStream.hpp"

namespace sn_TEST {
#define SN_TESTCASE(Name) \
	extern void sn_test_case_function_##Name(void); \
	struct SN_TestCaseClass_##Name { \
		SN_TestCaseClass_##Name() { \
			SN_LOG_DEBUG << #Name << std::nl; \
			sn_test_case_function_##Name(); \
		} \
	} sn_test_case_instance_##Name; \
	void sn_test_case_function_##Name(void)

#define SN_TEST(Name) \
	sn_test_case_instance_##Name();

}







#endif