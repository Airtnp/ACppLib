#ifndef SN_FUNCTION_TRAMPOLINE_H
#define SN_FUNCTION_TRAMPOLINE_H

#include <utility>
#include <type_traits>
#include <tuple>
#include <initializer_list>

namespace sn_Function {
    // ref: https://codereview.stackexchange.com/questions/161875/c-constexpr-trampoline
	namespace trampoline {
		template <typename T>
		constexpr auto trampoline(T&& init) {
			std::decay_t<T> answer{ std::forward<T>(init) };
			while (!answer.finished())
				answer = answer.tail_call();
			return answer.value();
		}

		template <typename F, typename Args>
		class TrampolineFunc {
			F m_func;
			Args m_args;
		public:
			using args_t = Args;
			constexpr TrampolineFunc() = default;
			constexpr TrampolineFunc(TrampolineFunc&& other) = default;
			constexpr TrampolineFunc(const TrampolineFunc& other) = default;
			constexpr TrampolineFunc& operator=(TrampolineFunc rhs) {
				m_func = std::move(rhs);
				assign_args(std::move(rhs.m_args), std::make_index_sequence<std::tuple_size<Args>::value>{});
			}
			constexpr TrampolineFunc(F&& fn, Args&& args)
				: m_func(std::forward<F>(fn), std::forward<Args>(args)) {}
			constexpr auto operator()() const {
				return m_func(std::forward<Args>(m_args));
			}
		private:
			template <std::size_t ...I>
			constexpr void assign_args(Args args, std::index_sequence<I...>) {
				std::initializer_list<int>{(std::get<I>(m_args) = std::get<I>(args), 0)...};
			}
		};

		template <typename F, typename Args>
		constexpr auto make_trampoline_func(F&& fn, Args&& args) {
			return TrampolineFunc<F, Args>(std::forward<F>(fn), std::forward<Args>(args));
		}

		template <typename T, typename U>
		class TrampolineWrapper {
			using F = TrampolineFunc<TrampolineWrapper<T, U>(*)(U), U>;
			bool m_finished;
			F m_func;
			T m_value;
		public:
			constexpr TrampolineWrapper() = default;
			constexpr TrampolineWrapper(TrampolineWrapper&&) = default;
			constexpr TrampolineWrapper(const TrampolineWrapper&) = default;
			
			constexpr TrampolineWrapper(bool finished, F&& fn, T value)
				: m_finished(finished), m_func(fn), m_value(value) {}
			constexpr bool finished() const {
				return m_finished;
			}
			constexpr T value() const {
				return m_value;
			}
			constexpr TrampolineWrapper tail_call() const {
				return m_func();
			}
		};

		template <typename T, typename F>
		constexpr auto make_trampoline_wrapper(bool finished, T&& value, F&& fn) {
			return TrampolineWrapper<std::decay_t<T>, typename F::args_t>{
				finished, std::forward<F>(fn), std::forward<T>(value)
			};
		}

		template <typename F, typename T, typename ...Args>
		constexpr auto make_trampoline(F&& fn, T&& init, Args&&... args) {
			return trampoline(
				make_trampoline_wrapper(
					false,
					std::forward<T>(init),
					make_trampoline_func(
						std::forward<F>(fn),
						std::make_tuple(std::forward<Args>(args)...)
					)
				)
			);
		}

		/*
		Usage:
			constexpr auto F(std::tuple<Args...> args) {
				// By condition
				return make_trampoline_wrapper<T, std::tuple<Args...>>(...);
				-> true
				-> false
			}
		*/
	}
}


#endif