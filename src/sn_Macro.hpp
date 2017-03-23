#ifndef SN_MACRO_H
#define SN_MACRO_H

#include "sn_CommonHeader.h"

namespace sn_Macro { //useless namespace

	//solve VS2015 macro expansion bugs

#define MACRO_EXPAND(...) __VA_ARGS__
#define APPLY_VARIADIC_MACRO(macro, ...) MACRO_EXPAND(macro(__VA_ARGS__))

#define MACRO_CONCAT(A, B) MACRO_CONCAT_IMPL(A, B)
#define MACRO_CONCAT_IMPL(A, B) A##_##B


					 //std::experimental::is_detected
					 //__if_exists in VS
					 //Usage: sn_has_member(a); bool has_member = sn_has_member_value<T>(a);
#define sn_has_member(member_name) \
	template <typename T, typename V = std::void_t<>> \
	struct sn_has_member_##member_name : std::false_type {}; \
	template <typename T> \
	struct sn_has_member_##member_name<T, std::void_t<decltype(T::##member_name)>> : std::true_type {};

#define sn_has_member_value(Type, member_name) \
	sn_has_member_##member_name<Type>::value

#define sn_has_member_function(func_name) \
	template <typename C, typename R, typename V = std::void_t<>, typename... Args> \
	struct sn_has_member_function_##func_name : std::false_type {}; \
	template <typename C, typename R, typename... Args> \
	struct sn_has_member_function_##func_name<C, R, \
		std::enable_if< \
			std::is_same< \
				R, decltype(std::declval<T>().##func_name(std::declval(Args>()...))) \
			>::value \
		>, Args...> : std::true_type {};


	/*
#define sn_has_member_function(func_name) \
	template <typename, typename T> \
	struct sn_has_member_function_##func_name { \
	}; \
	template <typename C, typename R, typename... Args> \
	struct sn_has_member_function_##func_name<C, R(Args...)> { \
		template <typename T> \
		static constexpr auto check(T*) \
		-> typename std::is_same<decltype(std::declval<T>().##func_name(std::declval<Args>()...)), R>::type; \
		\
		template <typename> \
		static constexpr auto check(...) \
		-> std::false_type; \
		\
		using type = decltype(check<C>(nullptr)); \
	};
	*/

#define sn_has_member_function_value(cls, func_name, ret, ...) \
	sn_has_member_function_##func_name<cls, ret(__VA_ARGS__)>::type::value;

}

#endif