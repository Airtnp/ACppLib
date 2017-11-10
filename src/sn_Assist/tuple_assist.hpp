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

		template<
			typename Tuple,
			typename Indices = std::make_index_sequence<std::tuple_size<Tuple>::value>>
		struct runtime_get_func_table;
		
		template<typename Tuple, size_t ...Indices>
		struct runtime_get_func_table<Tuple,std::index_sequence<Indices...>>{
			using return_type = typename std::tuple_element<0,Tuple>::type&;
			using get_func_ptr = return_type (*)(Tuple&) noexcept;
			static constexpr get_func_ptr table[std::tuple_size<Tuple>::value] = {
				&std::get<Indices>...
			};
		};
		
		template<typename Tuple, size_t ...Indices>
		constexpr typename
		runtime_get_func_table<Tuple,std::index_sequence<Indices...>>::get_func_ptr
		runtime_get_func_table<Tuple,std::index_sequence<Indices...>>::table[std::tuple_size<Tuple>::value];
		
		template<typename Tuple>
		constexpr
		typename std::tuple_element<0,typename std::remove_reference<Tuple>::type>::type&
		runtime_get(Tuple&& t,size_t index){
			using tuple_type =t ypename std::remove_reference<Tuple>::type;
			if(index >= std::tuple_size<tuple_type>::value)
				throw std::runtime_error("Out of range");
			return runtime_get_func_table<tuple_type>::table[index](t);
		}

		template <size_t I>
		struct visit_impl
		{
			template <typename T, typename F>
			static void visit(T& tup, size_t idx, const F& fun) {
				if (idx == I - 1) 
					fun(std::get<I - 1>(tup));
				else 
					visit_impl<I - 1>::visit(tup, idx, fun);
			}
		};
		 
		template <>
		struct visit_impl<0> {
			template <typename T, typename F>
			static void visit(T& tup, size_t idx, F fun) { assert(false); }
		};
		 
		template <typename F, typename... Ts>
		void visit_at(std::tuple<Ts...> const& tup, size_t idx, const F& fun) {
			visit_impl<sizeof...(Ts)>::visit(tup, idx, fun);
		}
		 
		template <typename F, typename... Ts>
		void visit_at(std::tuple<Ts...>& tup, size_t idx, const F& fun) {
			visit_impl<sizeof...(Ts)>::visit(tup, idx, fun);
		}

		// tuple cannot have same type
		template <std::size_t I, typename ...Args>
		std::variant<Args...> tuple_index_impl(size_t n, std::tuple<Args...>& tp) {
			if constexpr (I >= sizeof...(Args)) {
				throw std::out_of_range("Out of bound.");
				return {};
			} else {    
				if (n == I) {
					return std::get<I>(tp);
				}
				return tuple_index_impl<I + 1, Args...>(n, tp);
			}
		}
		
		// The result shall use std::visit
		template <typename ...Args>
		auto tuple_index(size_t n, std::tuple<Args...>& tp) {
			return tuple_index_impl<0, Args...>(n, tp);
		}
	}
}