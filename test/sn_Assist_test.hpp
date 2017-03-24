#ifndef SN_TEST_ASSIST_H
#define SN_TEST_ASSIST_H

#include "sn_CommonHeader_test.h"

namespace sn_Assist_test {
	
	template <typename T>
	struct is_iterator {
		constexpr static bool value = sn_Assist::sn_detect::sn_is_detected<std::iterator_traits, T>::value;
	};

	struct fake_iterator {
		using FI = fake_iterator;
		FI operator++(int) {
			return *this;
		}
	};

	struct concept_iterator {
		template <typename T>
		constexpr auto require(T it) -> decltype(it++) { return it; }
	};

	template <typename T>
	SN_REQUIRE(T, concept_iterator(T)) next_iterator(T it) { return it++; }

	template <typename T>
	SN_REQUIRE_NOT(void, concept_iterator(T)) next_iterator(T it) {}

	void sn_assist_test() {
		std::vector<int> v{ 1, 2, 3 };
		std::cout << sn_Assist::sn_type_descriptor::type_descriptor<long long int * const[20]>::descript() << std::endl;
		std::cout << is_iterator<std::vector<int>::iterator>::value << std::endl;
		constexpr auto b = SN_REQUIRE_VALUE(concept_iterator(std::vector<int>::iterator));
		auto p = next_iterator(v.begin());
	}
}

#endif