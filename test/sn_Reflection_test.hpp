#ifndef SN_TEST_REFLECTION_H
#define SN_TEST_REFLECTION_H

#include "sn_CommonHeader_test.h"


namespace sn_Reflection_test {
	struct foo {
		int f0;
		char f2[2];
	};

	namespace sn_reflection_detail {
		template <typename T>
		struct sn_reflection_member {};
		SN_REFLECTION(foo, f0, f2)
		
	}
	
	void sn_reflection_test() {
		using namespace sn_Assist::sn_tuple_assist;
		foo bar{ 1, {'a', '\0'} };
		std::cout << sn_reflection_detail::sn_reflection_member<foo>::arr[0] << std::endl;
		//std::cout << std::type_index(typeid(sn_reflection_detail::sn_reflection_member<foo>::arr[0])).name() << std::endl;
		auto p = std::get<1>(sn_reflection_detail::sn_reflection_member<foo>::apply());
		std::cout << bar.*p;
		//std::cout << static_cast<std::size_t>(sn_Reflection::pod_reflect::get<1>(bar));
	}
}


#endif