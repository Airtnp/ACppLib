#ifndef SN_TEST_ASSIST_H
#define SN_TEST_ASSIST_H

#include "sn_CommonHeader_test.h"

namespace sn_Assist_test {
	void sn_assist_test() {
		std::cout << sn_Assist::sn_type_descriptor::type_descriptor<long long int * const[20]>::descript() << std::endl;
	}
}

#endif SN_TEST_ALG_H