#include "../sn_CommonHeader.h"

namespace sn_Assist {

    namespace sn_numeric_assist {
		template <std::size_t arg1, std::size_t ...args>
		struct max_integer;

		template <std::size_t arg>
		struct max_integer<arg> : std::integral_constant<std::size_t, arg> {};

		template <std::size_t arg1, std::size_t arg2, std::size_t ...args>
		struct max_integer<arg1, arg2, args...> : 
			std::integral_constant<std::size_t, arg1 >= arg2 ? max_integer<arg1, args...>::value : max_integer<arg2, args...>::value> {};

		template <typename ...Args>
		struct max_align : std::integral_constant<std::size_t, max_integer<std::alignment_of<Args>::value...>::value> {};

	}

	namespace sn_type_assist {

		template <typename ...Args>
		struct identity {};

		template <>
		struct identity<> {};

		template <typename T, typename ...Args>
		struct identity<T, Args...> {
			using type = T;
		};

		template <typename T, typename ...Args>
		struct is_contain;

		template <typename T, typename H, typename ...Args>
		struct is_contain<T, H, Args...> : std::conditional<std::is_same<T, H>::value, std::true_type, is_contain<T, Args...>>::type {};

		template <typename T>
		struct is_contain<T> : std::false_type {};

		template <typename T, typename ...Args>
		struct contain_index;

		template <typename T, typename H, typename ...Args>
		struct contain_index<T, H, Args...> {
			enum {
				value = contain_index<T, Args...>::value + 1,
			};
		};

		template <typename T, typename ...Args>
		struct contain_index<T, T, Args...> {
			enum {
				value = 0,
			};
		};

		template <typename T>
		struct contain_index<T> {
			enum {
				value = -1,
			};
		};

		template <std::size_t index, typename ...Args>
		struct visit_at;

		template <std::size_t index, typename H, typename ...Args>
		struct visit_at<index, H, Args...> {
			using type = typename visit_at<index - 1, Args...>::type;
		};

		template <typename T, typename ...Args>
		struct visit_at<0, T, Args...> {
			using type = T;
		};



	}

	namespace sn_varadic {
		template<std::size_t N, std::size_t ...I>
		struct  append_index
		{
			using result = typename append_index<N - 1, N - 1, I...>::result;
		};

		template<std::size_t ...I>
		struct  append_index<0, I...>
		{
			using result = std::integer_sequence<std::size_t, I...>;
		};

		template <std::size_t ...I>
		constexpr void print_index(std::index_sequence<I...>) {
			int a[sizeof...(I)] = { (std::cout << I, 0)... };
		}

	}
}