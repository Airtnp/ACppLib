#include "../sn_CommonHeader.h"

namespace sn_Assist {
    namespace sn_tuple_assist {
		// This is just C++17 apply without ADL defense
		// VS2015 constexpr...
		template<typename F, typename T, std::size_t... I>
		/*constexpr*/ decltype(auto) invoke_impl(F&& func, T&& t, std::index_sequence<I...>)
		{
			return func(std::get<I>(std::forward<T>(t))...);
		}

		template<typename F, typename T>
		/*constexpr*/ decltype(auto) invoke_tuple(F&& func, T&& t)
		{
			constexpr auto size = std::tuple_size<typename std::decay<T>::type>::value;
			return invoke_impl(std::forward<F>(func), std::forward<T>(t), std::make_index_sequence<size>{});
		}

		template <typename T, typename Tp, std::size_t ...I>
		/*constexpr*/ T make_from_tuple_impl(Tp&& t, std::index_sequence<I...>) {
			return T(std::get<I>(std::forward<Tp>(t))...);
		}

		template <typename T, typename Tp>
		/*constexpr*/ T make_from_tuple(Tp&& t) {
			return make_from_tuple_impl<T>(std::forward<Tp>(t), std::make_index_sequence<std::tuple_size<std::decay_t<Tp>>>{});
		}


	}
}