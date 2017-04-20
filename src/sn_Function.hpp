#ifndef SN_FUNCTION_H
#define SN_FUNCTION_H

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"
#include "sn_Type.hpp"

// ref: https://github.com/vczh-libraries/Release/blob/master/Import/Vlpp.h (Change ref by license)
// ref: qicsomos/cosmos/Lazy qicosmos/cosmos/modern_functor
namespace sn_Function {
	
	namespace function {
		template <typename T>
		class Func {};

		namespace invoker {
			template <typename R, typename ...Args>
			class Invoker {
			public:
				virtual R invoke(Args&& ...args) = 0;
			};

			template <typename R, typename ...Args>
			class StaticInvoker : public Invoker<R, Args...> {
			protected:
				R(*m_func)(Args... args);
			public:
				StaticInvoker(R(*func)(Args...)) : m_func(func) {}
				R invoke(Args&& ...args) override {
					return m_func(std::forward<Args>(args)...);
				}
			};

			template <typename C, typename R, typename ...Args>
			class MemberInvoker : public Invoker<R, Args...> {
			protected:
				C* m_class;
				R(C::*m_func)(Args... args);
			public:
				MemberInvoker(C* _class, R(C::*func)(Args...)) : m_class(_class), m_func(func) {}
				R invoke(Args&& ...args) override {
					return (m_class->*m_func)(std::forward<Args>(args)...);
				}
			};

			template <typename C, typename R, typename ...Args>
			class CallableInvoker : public Invoker<R, Args...> {
			protected:
				C m_callable;
			public:
				CallableInvoker(const C& callable) : m_callable(callable) {}
				R invoke(Args&& ...args) override {
					return m_callable(std::forward<Args>(args)...);
				}
			};

			template <typename C, typename ...Args>
			class CallableInvoker<C, void, Args...> : public Invoker<void, Args...> {
			protected:
				C m_callable;
			public:
				CallableInvoker(const C& callable) : m_callable(callable) {}
				void invoke(Args&& ...args) override {
					m_callable(std::forward<Args>(args)...);
				}
			};


		}

		template <typename R, typename ...Args>
		class Func<R(Args...)> {
		protected:
			observer_ptr<invoker::Invoker<R, Args...>> m_invoker;
		public:
			typedef R function_type(Args...);
			using result_type = R;

			Func() {}
			Func(const Func<R(Args...)>& rhs) : m_invoker(rhs.m_invoker) {}
			Func(R(*func)(Args...)) {
				m_invoker = new invoker::StaticInvoker<R, Args...>(func);
			}
			template <typename C>
			Func(C* obj, R(C::*func)(Args...)) {
				m_invoker = new invoker::MemberInvoker<C, R, Args...>(obj, func);
			}
			template <typename C>
			Func(const C& func) {
				m_invoker = new invoker::CallableInvoker<C, R, Args...>(func);
			}

			R operator()(Args&&... args) const {
				return m_invoker->invoke(std::forward<Args>(args)...);
			}

			bool operator==(const Func<R(Args...)>& rhs) const {
				return m_invoker == rhs.m_invoker;
			}

			bool operator!=(const Func<R(Args...)>& rhs) const {
				return m_invoker != rhs.m_invoker;
			}

			operator bool() const {
				return m_invoker;
			}

		};

		template <typename R, typename ...Args>
		Func<R(Args...)> make_func(R(*function)(Args...)) {
			return Func<R(Args...)>(function);
		}

		template <typename C, typename R, typename ...Args>
		Func<R(Args...)> make_func(C* obj, R(C::*function)(Args...)) {
			return Func<R(Args...)>(obj, function);
		}

		template <typename R, typename ...Args>
		Func<R(Args...)> make_func(const R(&function)(Args...)) {
			return Func<R(Args...)>(function);
		}

		template <typename C>
		auto make_func(const C& function) {
			using T = sn_Assist::sn_function_traits::function_traits<C>;
			using FT = typename T::function_type;
			return Func<FT>(function);
		}

	}

	namespace bind {
		template <typename T, typename R, typename ...Args>
		auto member_lambda_bind(R(T::* const m_fun)(Args...), const T* obj) {
			return [=](auto&& ...args) {
				(obj->*m_fun)(std::forward<decltype(args)>(args)...);
			};
		}

#ifdef _MSC_VER
		template <typename T, typename R, typename ...Args, std::size_t Is>
		auto member_stl_bind_impl(R(T::* const m_fun)(Args...), const T* obj, std::index_sequence<Is...>) {
			return std::bind(m_fun, obj, std::_Ph<Is + 1>{}...);
		}

		template <std::size_t I, typename T, typename R, typename ...Args>
		auto member_stl_bind(R(T::* const m_fun)(Args...), const T* obj) {
			return member_stl_bind_impl(m_fun, obj, std::make_index_sequence<I>{});
		}
#endif
	}

	namespace currying {
		template <typename T>
		struct Currying {};


		template <typename R, typename Arg0, typename ...Args>
		struct Currying<R(Arg0, Args...)> {
			typedef R function_type(Arg0, Args...);
			typedef R curried_type(Args...);
			using first_parameter_type = Arg0;

			class Binder {
			protected:
				function::Func<function_type> m_target;
				Arg0 m_firstParam;
			public:
				Binder(const function::Func<function_type>& target, Arg0 param) : m_target(target), m_firstParam(param) {}

				R operator()(Args&&... args) {
					return m_target(std::forward<Arg0>(m_firstParam), std::forward<Args>(args)...);
				}

				function::Func<function_type> temp_func() {
					return m_target;
				}
			};

			class Currier {
			protected:
				function::Func<function_type> m_target;
			public:
				Currier(const function::Func<function_type>& target) : m_target(target) {}
				template <typename RT, typename ...TArgs>
				Currier(const typename Currying<RT(Args...)>::Binder& binder) : m_target(binder) {}

				typename Currying<R(Args...)>::Currier operator()(Arg0&& param) const {
					return typename Currying<R(Args...)>::Currier(Binder(m_target, param));
				}

			};

			class SingleCurrier {
			protected:
				function::Func<function_type> m_target;
			public:
				SingleCurrier(const function::Func<function_type>& target) : m_target(target) {}

				Binder operator()(Arg0&& param) const {
					return Binder(m_target, param);
				}

			};

			template <std::size_t N, typename V = void>
			class MultiCurrier {
			protected:
				function::Func<function_type> m_target;
			public:
				MultiCurrier(const function::Func<function_type>& target) : m_target(target) {}
				template <typename RT, typename ...TArgs>
				MultiCurrier(const typename Currying<RT(Args...)>::Binder& binder) : m_target(binder) {}

				typename Currying<R(Args...)>::template MultiCurrier<N - 1> operator()(Arg0&& param) const {
					return typename Currying<R(Args...)>::template MultiCurrier<N - 1>(Binder(m_target, param));
				}
			};

			template <typename V>
			class MultiCurrier<1, V> {
			protected:
				function::Func<function_type> m_target;
			public:
				MultiCurrier(const function::Func<function_type>& target) : m_target(target) {}

				Binder operator()(Arg0&& param) const {
					return Binder(m_target, param);
				}
			};
		};

		template <typename R, typename Arg0>
		struct Currying<R(Arg0)> {
			typedef R function_type(Arg0);
			typedef R curried_type(Arg0);
			using first_parameter_type = Arg0;

			class Binder {
			protected:
				function::Func<function_type> m_target;
				Arg0 m_firstParam;
			public:
				Binder(const function::Func<function_type>& target, Arg0 param) : m_target(target), m_firstParam(param) {}

				R operator()() {
					return m_target(std::forward<Arg0>(m_firstParam));
				}

				function::Func<function_type> temp_func() {
					return m_target;
				}
			};

			class Currier {
			protected:
				function::Func<function_type> m_target;
			public:
				Currier(const function::Func<function_type>& target) : m_target(target) {}

				Binder operator()(Arg0&& param) const {
					return Binder(m_target, param);
				}

			};

			class SingleCurrier {
			protected:
				function::Func<function_type> m_target;
			public:
				SingleCurrier(const function::Func<function_type>& target) : m_target(target) {}

				Binder operator()(Arg0&& param) const {
					return Binder(m_target, param);
				}

			};

			template <std::size_t N, typename = void>
			class MultiCurrier {
			protected:
				function::Func<function_type> m_target;
			public:
				MultiCurrier(const function::Func<function_type>& target) : m_target(target) {}

				Binder operator()(Arg0&& param) const {
					return Binder(m_target, param);
				}
			};

			template <typename V>
			class MultiCurrier<1, V> {
			protected:
				function::Func<function_type> m_target;
			public:
				MultiCurrier(const function::Func<function_type>& target) : m_target(target) {}

				Binder operator()(Arg0&& param) const {
					return Binder(m_target, param);
				}
			};
		};



		template <typename R, typename ...Args>
		typename Currying<R(Args...)>::Currier make_curry(R(*function)(Args...)) {
			return typename Currying<R(Args...)>::Currier(function);
		}

		template <typename C, typename R, typename ...Args>
		typename Currying<R(Args...)>::Currier make_curry(C* obj, R(C::*function)(Args...)) {
			return typename Currying<R(Args...)>::Currier(function::Func<R(Args...)>(obj, function));
		}

		template <typename R, typename ...Args>
		typename Currying<R(Args...)>::Currier make_curry(const R(&function)(Args...)) {
			return typename Currying<R(Args...)>::Currier(function);
		}

		template <typename R, typename ...Args>
		typename Currying<R(Args...)>::Currier make_single_curry(R(*function)(Args...)) {
			return typename Currying<R(Args...)>::SingleCurrier(function);
		}

		template <typename C, typename R, typename ...Args>
		typename Currying<R(Args...)>::Currier make_single_curry(C* obj, R(C::*function)(Args...)) {
			return typename Currying<R(Args...)>::SingleCurrier(function::Func<R(Args...)>(obj, function));
		}

		template <typename R, typename ...Args>
		typename Currying<R(Args...)>::Currier make_single_curry(const R(&function)(Args...)) {
			return typename Currying<R(Args...)>::SingleCurrier(function);
		}

		template <std::size_t N, typename R, typename ...Args>
		typename Currying<R(Args...)>::Currier make_multi_curry(R(*function)(Args...)) {
			return typename Currying<R(Args...)>::template MultiCurrier<N>(function);
		}

		template <std::size_t N, typename C, typename R, typename ...Args>
		typename Currying<R(Args...)>::Currier make_multi_curry(C* obj, R(C::*function)(Args...)) {
			return typename Currying<R(Args...)>::template MultiCurrier<N>(function::Func<R(Args...)>(obj, function));
		}

		template <std::size_t N, typename R, typename ...Args>
		typename Currying<R(Args...)>::Currier make_multi_curry(const R(&function)(Args...)) {
			return typename Currying<R(Args...)>::template MultiCurrier<N>(function);
		}

		template <typename C>
		auto make_curry(const C& function) {
			using T = sn_Assist::sn_function_traits::function_traits<C>;
			using FT = typename T::function_type;
			return typename Currying<FT>::Currier(function);
		}

		template <typename C>
		auto make_single_curry(const C& function) {
			using T = sn_Assist::sn_function_traits::function_traits<C>;
			using FT = T::function_type;
			return typename Currying<FT>::SingleCurrier(function);
		}

		template <std::size_t N, typename C>
		auto make_multi_curry(const C& function) {
			using T = sn_Assist::sn_function_traits::function_traits<C>;
			using FT = T::function_type;
			return typename Currying<FT>::template MultiCurrier<N>(function);
		}

	}

	namespace combining {
		template <typename A, typename B, typename C>
		class Combining {};

		template <typename R1, typename R2, typename R, typename ...Args>
		class Combining<R1(Args...), R2(Args...), R(R1, R2)> {
		protected:
			function::Func<R1(Args...)> m_func1;
			function::Func<R2(Args...)> m_func2;
			function::Func<R(R1, R2)> m_converter;
		public:
			typedef R1 first_function_type(Args...);
			typedef R2 second_function_type(Args...);
			typedef R converter_function_type(R1, R2);
			typedef R function_type(Args...);

			using FT1 = first_function_type;
			using FT2 = second_function_type;
			using FCT = converter_function_type;
			using FT = function_type;


			Combining(const FT1& func1, const FT2& func2, const FCT& converter)
				: m_func1(func1)
				, m_func2(func2)
				, m_converter(converter) {}

			R operator()(Args&&... args) const {
				return m_converter(m_func1(std::forward<Args>(args)...), m_func2(std::forward<Args>(args)...));
			}

		};

		using function::Func;
		using currying::make_curry;

		template <typename F1, typename F2, typename C>
		Func<typename Combining<F1, F2, C>::function_type> make_combine(Func<C> converter, Func<F1> func1, Func<F2> func2) {
			return Combining<F1, F2, C>(func1, func2, converter);
		}

		template <typename T>
		Func<Func<T>(Func<T>, Func<T>)> make_homomorphy_combine(const Func<typename Func<T>::result_type(typename Func<T>::result_type, typename Func<T>::result_type)> converter) {
			using R = typename Func<T>::result_type;
			// Or use bind to make converter the 3rd argument
			return make_curry<Func<T>(Func<R(R, R)>, Func<T>, Func<T>)>(make_combine)(converter);
		}


	}

	namespace lazy {
		using function::Func;
		template <typename T, typename ...Args>
		class Lazy {
		protected:
			class Internal {
			public:
				Func<T(Args...)> m_evaluator;
				std::tuple<const Args&...> m_args;
				T m_value;
				bool m_isEvaluated;
			};
			observer_ptr<Internal> m_internal = nullptr;
		public:
			Lazy() {}
			Lazy(const Func<T(Args...)>& eval, Args&&... args) {
				m_internal = new Internal;
				m_internal->m_isEvaluated = false;
				m_internal->m_evaluator = eval;
				m_internal->m_args = std::make_tuple(std::forward<Args>(args)...);
			}
			Lazy(const T& value) {
				m_internal = new Internal;
				m_internal->m_isEvaluated = true;
				m_internal->m_value = value;
			}
			Lazy(const Lazy& rhs) : m_internal(rhs.m_internal) {}
			Lazy(Lazy&& rhs) : m_internal(std::move(m_internal)) {}

			/*
			Lazy& operator=(const Func<T(Args...)>& eval, Args&&... args) {
				m_internal = new Internal;
				m_internal->m_isEvaluated = false;
				m_internal->m_evaluator = eval;
				m_internal->m_args = std::make_tuple(std::forward<Args>(args)...);
				return *this;
			}*/
			
			Lazy& operator=(const T& value) {
				m_internal = new Internal;
				m_internal->m_isEvaluated = true;
				m_internal->m_value = value;
				return *this;
			}

			Lazy& operator=(const Lazy& rhs) {
				m_internal = new Internal;
				m_internal->m_isEvaluated = rhs.m_internal->m_isEvaluated;
				m_internal->m_value = rhs.m_internal->m_value;
				m_internal->m_evaluator = rhs.m_internal->m_evaluator;
				m_internal->m_args = rhs.m_internal->m_args;
				return *this;
			}
			Lazy& operator=(Lazy&& rhs) {
				m_internal = std::move(rhs.m_internal);
				rhs.m_internal = nullptr;
				return *this;
			}

			const T& value() const {
				if (m_internal != nullptr) {
					if (!m_internal->m_isEvaluated) {
						m_internal->m_isEvaluated = true;
						m_internal->m_value = sn_Assist::sn_tuple_assist::invoke_tuple(m_internal->m_evaluator, m_internal->m_args);
					}
					return m_internal_value;
				}
				throw std::bad_exception("Uninitialized internal value");
			}

			const bool is_evaluated() const {
				return m_internal->m_isEvaluated();
			}

			const bool is_available() const {
				return m_internal != nullptr;
			}
		};

		template <typename T>
		Lazy<T> make_lazy(T&& value) {
			return Lazy(std::forward<T>(value));
		}

		template <typename R, typename ...Args>
		Lazy<R, Args...> make_lazy(Func<R(Args...)>&& func, Args&&... args) {
			return Lazy(std::forward<Func(R(Args...))> func, std::forward<Args>(args)...);
		}

	}

	//Note: functor wrapper is just like AOP
	namespace functor_wrapper {

#define SN_MAKE_FUNCTOR(func_name) \
	class sn_fn_##func_name { \
	public : \
		template <typename ...Args> \
		auto operator()(Args&&... args) const { \
			return func_name(std::forward<Args>(args)...); \
		} \
	} \


		template <typename F, typename Before = std::tuple<>, typename After = std::tuple<>>
		class FunctorWrapper {
		private:
			F m_func;
			Before m_before;
			After m_after;
		public:
			FunctorWrapper(F&& f) : m_func(std::forward<F>(f)), m_before(std::tuple<>()), m_after(std::tuple<>()) {}
			FunctorWrapper(const F& f, const Before& b, const After& a) : m_func(f), m_before(b), m_after(a) {}

			template <typename ...Args>
			auto operator()(Args&&... args) const
				-> decltype(sn_Assist::sn_tuple_assist::invoke_tuple(
					m_func, std::tuple_cat(
						m_before,
						std::make_tuple(std::forward<Args>(args)...),
						m_after))) {
				return sn_Assist::sn_tuple_assist::invoke_tuple(m_func, std::tuple_cat(m_before, std::make_tuple(std::forward<Args>(args)...), m_after));
			}

			template <typename T>
			auto add_aspect_head(T&& param) const {
				using Before_t = decltype(std::tuple_cat(std::make_tuple(std::forward<T>(param)), m_before));
				return FunctorWrapper<F, Before_t, After>(m_func, std::tuple_cat(std::make_tuple(std::forward<T>(param)), m_before), m_after);
			}

			template <typename T>
			auto add_aspect_inplace(T&& param) const {
				using Before_t = decltype(std::tuple_cat(m_before, std::make_tuple(std::forward<T>(param))));
				return FunctorWrapper<F, Before_t, After>(m_func, std::tuple_cat(m_before, std::make_tuple(std::forward<T>(param))), m_after);
			}

			template <typename T>
			auto add_aspect_tail(T&& param) const {
				using After_t = decltype(std::tuple_cat(std::make_tuple(std::forward<T>(param)), m_after));
				return FunctorWrapper<F, Before, After_t>(m_func, m_before, std::tuple_cat(std::make_tuple(std::forward<T>(param)), m_after));
			}
		};

		template <typename F>
		auto make_functor_wrapper(F&& f) {
			return FunctorWrapper<F>(std::forward<F>(f));
		}

		// 120 >> 3 >> wrapper_map << 2 << 3 << 20
		// (wrapper_map + 2 + 3) << 4
		// 1 | (add << 4 << 6) | print = 11
		template <typename F, typename Arg>
		auto operator+(const FunctorWrapper<F>& pa, Arg&& arg) {
			return pa.template add_aspect_inplace<Arg>(std::forward<Arg>(arg));
		}

		template <typename F, typename Arg>
		auto operator<<(const FunctorWrapper<F>& pa, Arg&& arg) {
			return pa.template add_aspect_tail<Arg>(std::forward<Arg>(arg));
		}

		template <typename Arg, typename F>
		auto operator>>(Arg&& arg, const FunctorWrapper<F>& pa) {
			return pa.template add_aspect_head<Arg>(std::forward<Arg>(arg));
		}

#define SN_MAKE_FUNCTOR_WRAPPER(wrapper_name, func_name) \
	SN_MAKE_FUNCTOR(func_name) \
	const auto wrapper_name = make_functor_wrapper(sn_fn_##func_name());


	}

	namespace maybe_just {
		template <typename T>
		class maybe {
		private:
			sn_Type::optional::Optional<T> m_optional;
		public:
			maybe() : m_optional() {}
			maybe(T&& value) : m_optional(std::forward<T>(value)) {}
			maybe(const T& value) : m_optional(value) {}

			template <typename F>
			auto operator()(F&& f) const -> maybe<decltype(f(std::declval<T>()))> {
				using result_type = decltype(f(std::declval<T>()));
				if (!m_optional.is_init())
					return maybe<result_type>();
				return maybe<result_type>(f(m_optional.value()));
			}

			template <typename U>
			T value_or(U&& another_value) {
				return m_optional.value_or(std::forward<U>(another_value));
			}
		};

		// maybe<int>() | print || 2 -> 2
		// just(2) | print || 4 -> print(4)
		template <typename T, typename F>
		inline auto operator|(maybe<T>&& m, F&& f) -> decltype(m(f)) {
			return m(std::forward<F>(f));
		}

		template <typename T, typename U>
		inline auto operator||(maybe<T>&& m, U&& v) -> decltype(m.value_or(std::forward<U>(v))) {
			return m.value_or(std::forward<U>(v));
		}

		template <typename T>
		maybe<T> just(T&& t) {
			return maybe<T>(std::forward<T>(t));
		}

	}

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
			return Chain.add(std::forward<F>(f));
		}

	}

	namespace operation {

		template <typename T, typename ...Args, template <typename...> typename C, typename F>
		auto map(const C<T, Args...>& container, const F& f) -> C<decltype(f(std::declval<T>()))> {
			using result_type = decltype(f(std::declval<T>()));
			C<result_type> res;
			for (const auto& item : container)
				res.push_back(f(item));
			return res;
		}

		template <typename T, typename ...Args, template <typename...> typename C, typename F, typename R>
		auto reduce(const C<T, Args...>& container, const R& start, const F& f) {
			R res = start;
			for (const auto& item : container)
				res = f(res, item);
			return res;
		}

		template <typename T, typename ...Args, template <typename...> typename C, typename F>
		auto filter(const C<T, Args...>& container, const F& f) {
			C<T, Args...> res;
			for (const auto& item : container)
				if (f(item))
					res.push_back(item);
			return res;
		}

	}

	using function::make_func;
	using currying::make_curry;
	using currying::make_single_curry;
	using currying::make_multi_curry;
	using combining::make_combine;
	using combining::make_homomorphy_combine;
	using maybe_just::maybe;
	using maybe_just::just;
	using lazy::make_lazy;
	using functor_wrapper::make_functor_wrapper;

	const auto ChainHead = pipeline::Chain<>();
}





#endif