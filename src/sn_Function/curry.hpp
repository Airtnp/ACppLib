#ifndef SN_FUNCTION_CURRY_H
#define SN_FUNCTION_CURRY_H

#include <utility>
#include <tuple>
#include <type_traits>
#include "../sn_Assist.hpp"

namespace sn_Function {
	// ref: https://vittorioromeo.info/index/blog/cpp17_curry.html
	// TODO: reduce capture
	namespace lambda_currying {
		
		template <typename TF, typename Tp>
		decltype(auto) for_tuple(TF&& f, Tp&& tp) {
			return sn_Assist::sn_tuple_assist::invoke_tuple(
				[&f](auto&&... xs) {
					std::initializer_list<int>{(std::forward<TF>(f)(std::forward<decltype(xs)>(xs)), 0)...};
				},
				std::forward<Tp>(tp)
			);
		}

		template <typename T>
		class forward_capture_tuple : private std::tuple<T> {
		private:
			using decay_t = std::decay_t<T>;
			using base_t = std::tuple<T>;
		protected:
			constexpr auto& as_tuple() const noexcept {
				return static_cast<base_t&>(*this);
			}
			constexpr const auto& as_tuple() noexcept {
				return static_cast<const base_t&>(*this);
			}
			template <typename TF, typename V = std::enable_if_t<!std::is_same<std::decay_t<TF>, forward_capture_tuple>::value>>
			constexpr forward_capture_tuple(TF&& x) noexcept(std::is_nothrow_constructible<base_t, TF>::value)
				: base_t(std::forward<TF>(x)) {}
		public:
			constexpr auto& get() & noexcept {
				return std::get<0>(as_tuple());
			}
			constexpr const auto& get() const & noexcept {
				return std::get<0>(as_tuple());
			}
			constexpr auto&& get() && noexcept(std::is_move_constructible<decay_t>::value) {
				return std::move(std::get<0>(as_tuple()));
			}	
		};

		template <typename T>
		class forward_capture_wrapper : public forward_capture_tuple<T> {
		private:
			using base_t = forward_capture_tuple<T>;
		public:
			template <typename TF, typename V = std::enable_if_t<!std::is_same<std::decay_t<TF>, forward_capture_wrapper>::value>>
			constexpr forward_capture_wrapper(TF&& x) noexcept(std::is_nothrow_constructible<base_t, TF>::value)
				: base_t(std::forward<TF>(x)) {}
		};

		template <typename T>
		class forward_copy_capture_wrapper : public forward_capture_tuple<T> {
		private:
			using base_t = forward_capture_tuple<T>;
		public:
			template <typename TF>
			constexpr forward_copy_capture_wrapper(TF&& x) noexcept(std::is_nothrow_constructible<base_t, TF>::value)
				: base_t(x) {}
		};

		template <typename T>
		constexpr auto make_forward_capture(T&& x)
		    noexcept(forward_capture_wrapper<T>::forward_capture_wrapper(std::declval<T>())) {
			return forward_capture_wrapper<T>(std::forward<T>(x));
		}
		template <typename T>
		constexpr auto make_forward_copy_capture(T&& x)
		    noexcept(forward_copy_capture_wrapper<T>::forward_copy_capture_wrapper(std::declval<T>())) {
			return forward_copy_capture_wrapper<T>(std::forward<T>(x));
		}

		template <typename ...Ts>
		constexpr auto make_forward_capture_as_tuple(Ts&&... xs)
		    noexcept(std::make_tuple(forward_capture_wrapper<Ts>::forward_capture_wrapper(std::declval<Ts>())...)) {
			return std::make_tuple(forward_capture_wrapper(std::forward<Ts>(xs))...);
		}
		template <typename ...Ts>
		constexpr auto make_forward_copy_capture_as_tuple(Ts&&... xs)
		    noexcept(std::make_tuple(forward_copy_capture_wrapper<Ts>::forward_copy_capture_wrapper(std::declval<Ts>())...)) {
			return std::make_tuple(forward_copy_capture_wrapper(std::forward<Ts>(xs))...);
		}

		template <typename TF, typename ...TFCs>
		constexpr decltype(auto) apply_forward_capture(TF&& f, TFCs&&... fcs) {
			return sn_Assist::sn_tuple_assist::invoke_tuple(
				[&f](auto&&... xs) -> decltype(auto) {
					return std::forward<TF>(f)(std::forward<decltype(xs)>(xs).get()...);
				},
				std::tuple_cat(std::forward<TFCs>(fcs)...)
			);
		}

		template <typename T, typename U>
		using copy_referenceness_impl =
			std::conditional_t<!std::is_reference<U>::value, T,
				std::conditional_t<std::is_lvalue_reference<U>{},
					std::add_lvalue_reference_t<T>,
					std::conditional_t<std::is_rvalue_reference<U>{},
						std::add_rvalue_reference_t<T>, void>>>;

		template <typename T, typename U>
		using as_if_forwarded = std::conditional_t<!std::is_reference<U>::value,
			std::add_rvalue_reference_t<std::remove_reference_t<T>>,
		    copy_referenceness_impl<T, U>>;

		// forwards the passed argument with the same value category of the potentially-unrelated specified type. It basically copies the "lvalue/rvalue-ness" of the user-provided template parameter and applies it to its argument.
		template <typename U, typename T>
		inline constexpr decltype(auto) forward_like(T&& x) noexcept {
			static_assert(!(std::is_rvalue_reference<T>::value &&
							std::is_lvalue_reference<U>::value));
			return static_cast<as_if_forwarded<T, U>>(x);
		}

		template <typename TF>
		constexpr decltype(auto) make_lambda_curry(TF&& f) {
			if constexpr (std::is_invocable_v<TF>) {
				return std::forward<TF>(f)();
			}
			else {
				return [xf = make_forward_capture(std::forward<TF>(f))](auto&&... ps) mutable constexpr {
					return make_lambda_curry(
						[
							partial_pack = make_forward_capture_as_tuple(std::forward<decltype(ps)>(ps)...),
							yf = std::move(xf)
						]
						(auto&&... xs) constexpr
							-> decltype(forward_like<TF>(xf.get())(std::forward<decltype(ps)>(ps)..., std::forward<decltype(xs)>(xs)...)) {
								return apply_forward_capture(
									[&yf](auto&&... ys) constexpr
										-> decltype(forward_like<TF>(yf.get())(std::forward<decltype(ys)>(ys)...)) {
										return forward_like<TF>(yf.get())(std::forward<decltype(ys)>(ys)...);
									},
									partial_pack, make_forward_capture_as_tuple(std::forward<decltype(xs)>(xs)...)
								);
						}
					);
				};
			}
		}	
	}

}

#endif