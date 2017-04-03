#ifndef SN_TEST_LOG_H
#define SN_TEST_LOG_H

#include "sn_CommonHeader_test.h"

namespace sn_Log_test {
	void sn_log_test() {
		SN_BASIC_LOG(std::cout, "test1");
		SN_BASIC_LOG(std::cout, "test2");
		SN_LOG_WARN << "test warning";
	}
}

#endif