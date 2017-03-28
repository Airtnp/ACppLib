#ifndef SN_LAZY_H
#define SN_LAZY_H

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"

namespace sn_Lazy {
	//ref: Vlpp/Lazy Vlpp/Function
	namespace Function {
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

	}

	namespace Currying {
		template <typename T>
		struct Currying {};

		template <typename R, typename Arg0, typename ...Args>
		struct Currying<R(Arg0, Args...)> {
			typedef R function_type(Arg0, Args...);
			typedef R curried_type(Args...);
			using first_parameter_type = Arg0;

			class Binder {
			protected:
				Function::Func<function_type> m_target;
				Arg0 m_firstParam;
			public:
				Binder(const Function::Func<function_type>& target, Arg0 param) : m_target(target), m_firstParam(param) {}

				R operator()(Args&&... args) const {
					return m_target(std::forward<Arg0>(m_firstParam), std::forward<Args>(args)...);
				}
			};

			class Currier {
			protected:
				Function::Func<function_type> m_target;
			public:
				Currier(const Function::Func<function_type>& target) : m_target(target) {}

				Function::Func<curried_type> operator()(Arg0&& param) const {
					return Binder(m_target, param);
				}

			};
		};

		template <typename T>
		Function::Func<Function::Func<typename Currying<T>::curried_type>(typename Binding<T>::first_parameter_type)> make_curry(T* function) {
			return typename Currying<T>::Currier(function);
		}

		template <typename T>
		Function::Func<Function::Func<typename Currying<T>::curried_type>(typename Binding<T>::first_parameter_type)> make_curry(const T& function) {
			return typename Currying<T>::Currier(function);
		}

	}

	namespace Combining {
		template <typename A, typename B, typename C>
		class Combining {};

		template <typename R1, typename R2, typename R, typename ...Args>
		class Combining<R1(Args...), R2(Args...), R(R1, R2)> {
		protected:
			Function::Func<R1(Args...)> m_func1;
			Function::Func<R2(Args...)> m_func2;
			Function::Func<R(R1, R2)> m_converter;
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

		using Function::Func;
		using Currying::make_curry;

		template <typename F1, typename F2, typename C>
		Func<typename Combining<F1, F2, C>::function_type> make_combine(Func<C> converter, Func<F1> func1, Func<F2> func2) {
			return Combining<F1, F2, C>(func1, func2, converter);
		}

		template <typename T>
		Func<Func<T>(Func<T>, Func<T>)> make_homomorphy_combine(const Func<typename Func<T>::result_type(typename Func<T>::result_type, typename Func<T>::result_type)> converter) {
			using R = typename Func<T>::result_type;
			// Or use bind to make converter the 3rd argument
			return make_curry<Func<T>(Func<R(R, R)>, Func<T>, Func<T>)>(combine)(converter);
		}


	}

	namespace Lazy {
		using Function::Func;
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

	using Currying::make_curry;
	using Combining::make_combine;
	using Combining::make_homomorphy_combine;
	using Lazy::make_lazy;

}





#endif