#ifndef SN_TEST_DECIMAL_H
#define SN_TEST_DECIMAL_H

#include "sn_CommonHeader_test.h"

namespace sn_Decimal_test {
	void sn_decimal_test() {
		sn_Decimal::sn_unsigned_decimal<sn_Decimal::sn_decimal_bit> d(10999);
		sn_Decimal::sn_unsigned_decimal<sn_Decimal::sn_decimal_bit> e(1);
		auto f = d + e;
		std::cout << f;
	}
}

#endif