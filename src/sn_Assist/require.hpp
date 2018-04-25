#include "../sn_CommonHeader.h"

namespace sn_Assist {
    // ref: https://github.com/akemimadoka/NatsuLib/blob/master/NatsuLib/natConcepts.h
	namespace sn_require {

		template <typename C, typename = void>
		struct Require : std::false_type {};

		template <typename Concept, typename ...Ts>
		struct Require<Concept(Ts...), std::void_t<decltype(std::declval<Concept>().require(std::declval<Ts>()...))>> : std::true_type {};

#define SN_REQUIRE_VALUE(...) \
	sn_Assist::sn_require::Require<__VA_ARGS__>::value

#define SN_REQUIRE(ReturnType, ...) \
	std::enable_if_t<sn_Assist::sn_require::Require<__VA_ARGS__>::value, ReturnType>

#define SN_REQUIRE_NOT(ReturnType, ...) \
	std::enable_if_t<!sn_Assist::sn_require::Require<__VA_ARGS__>::value, ReturnType>


#define SN_REQUIRE_RAW(ReturnType, ...) \
	std::enable_if_t<(__VA_ARGS__), ReturnType>


		/* Usage:
			struct Property {
				template <typename T>
				auto require(T&& v) -> decltype(++x);
			}
			template <typename T>
			SN_REQUIRE(void, Property<T>) foo(T& x) { ++x; }
		*/

	}
}