#ifndef SN_FUNCTION_PIPELINE_H
#define SN_FUNCTION_PIPELINE_H

#include <utility>
#include <tuple>
#include <type_traits>

namespace sn_Function {
	// ref: qicsomos/cosmos/Lazy qicosmos/cosmos/modern_functor
    namespace pipeline {
		// 2 | add
		template <typename T, typename F>
		auto operator|(T&& param, const F& f) -> decltype(f(std::forward<T>(param))) {
			return f(std::forward<T>(param));
		}

		template <typename ...Fns>
		class Chain {
		private:
			const std::tuple<Fns...> m_funcs;
			const static size_t m_tpSize = sizeof...(Fns);

			template <typename Arg, std::size_t I>
			auto call_impl(Arg&& arg, const std::index_sequence<I>&) const
				-> decltype(std::get<I>(m_funcs)(std::forward<Arg>(arg))) {
				return std::get<I>(m_funcs)(std::forward<Arg>(arg));
			}

			template <typename Arg, std::size_t I, std::size_t ...Is>
			auto call_impl(Arg&& arg, const std::index_sequence<I, Is...>&) const
				-> decltype(call_impl(std::get<I>(m_funcs)(std::forward<Arg>(arg)), std::index_sequence<Is...>{})) {
				return call_impl(std::get<I>(m_funcs)(std::forward<Arg>(arg)), std::index_sequence<Is...>{});
			}

			template <typename Arg>
			auto call(Arg&& arg) const -> decltype(call_impl(std::forward<Arg>(arg), std::make_index_sequence<m_tpSize>{})) {
				return call_impl(std::forward<Arg>(arg), std::make_index_sequence<m_tpSize>{});
			}

		public:

			Chain() : m_funcs(std::tuple<>{}) {}
			Chain(std::tuple<Fns...> funcs) : m_funcs(funcs) {}
			
			template <typename F>
			inline auto add(const F& f) const {
				return Chain<Fns..., F>(std::tuple_cat(m_funcs, std::make_tuple(f)));
			}

			template <typename Arg>
			inline auto operator()(Arg&& arg) const -> decltype(call(std::forward<Arg>(arg))) {
				return call(std::forward<Arg>(arg));
			}

		};

		// f = ChainHead | [](auto s){} | wrapper_map | ...
		// {2, 3} | f
		template <typename ...Fns, typename F>
		inline auto operator|(Chain<Fns...>&& chain, F&& f) {
			return chain.add(std::forward<F>(f));
		}

	}
	const auto ChainHead = pipeline::Chain<>();	
}


#endif