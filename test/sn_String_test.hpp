#ifndef SN_TEST_STRING_H
#define SN_TEST_STRING_H

#include "sn_CommonHeader_test.h"

namespace sn_String_test {
	void sn_string_test() {
		std::cout << sn_String::formatter::string_formatter("{0}, {1}", "abbb", "b") << std::endl;
	}
}



#endif