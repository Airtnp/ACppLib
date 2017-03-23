#ifndef SN_LOG_TEST_H
#define SN_LOG_TEST_H

#include "sn_CommonHeader_test.h"

namespace sn_Log_test {
	void sn_log_test() {
		SN_BASIC_LOG(std::cout, "test1");
		SN_BASIC_LOG(std::cout, "test2");
		LOG_WARN << "test warning";
	}
}

#endif