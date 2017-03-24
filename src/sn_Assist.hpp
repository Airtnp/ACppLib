#ifndef SN_ASSIST_H
#define SN_ASSIST_H

#include "sn_CommonHeader.h"

namespace sn_Assist {
	namespace sn_detect {
		template <typename T, T v>
		struct sn_integral_constant {
			static constexpr T value = v;
			using value_type = T;
			using type = sn_integral_constant;
			constexpr operator value_type() const noexcept { return value; }
			constexpr value_type operator()() const noexcept { return value; }
		};


		using sn_false_type = sn_integral_constant<bool, false>;
		using sn_true_type = sn_integral_constant<bool, false>;

		struct sn_nonesuch {
			sn_nonesuch() = delete;
			~sn_nonesuch() = delete;
			sn_nonesuch(sn_nonesuch const&) = delete;
			void operator= (sn_nonesuch const&) = delete;
		};

		template <class T, class = std::void_t<>, template<class...> class Op, class... Args>
		struct sn_detector {
			using value_t = std::false_type;
			using type = T;
		};

		template <class T, template<class...> class Op, class... Args>
		struct sn_detector<T, std::void_t<Op<Args...>>, Op, Args...> {
			using value_t = std::true_type;
			using type = Op<Args...>;
		};

		template <template<class...> class Op, class... Args>
		using sn_is_detected = typename sn_detector<sn_nonesuch, void, Op, Args...>::value_t;

		template <template<class...> class Op, class... Args>
		using sn_is_detected_v = typename sn_detector<sn_nonesuch, void, Op, Args...>::value_t::value_type;

		template <template<class...> class Op, class... Args>
		using sn_detected_v = typename sn_detector<sn_nonesuch, void, Op, Args...>::type;


		/*
		Usage:
		template<class T>
		using has_member_r = decltype(&T::r)

		struct Type {
			int r;
		}

		sn_is_detected<has_member_r, Type>::value_t == std::true_type::value_t == true
		*/

	}

	namespace sn_function_traits {
		template <typename T>
		struct function_traits;

		//simple function
		template <typename R, typename... Args>
		struct function_traits<R(Args...)> {
			enum {
				args_size = sizeof...(Args)
			};
			typedef R type(Args...);
			using result_type = R;
			using pointer = R(*)(Args...);
			typedef R function_type(Args...);
			using stl_function_type = std::function<function_type>;

			template <size_t I>
			struct args {
				using type = typename std::tuple_element<I, std::tuple<Args...>>::type;
			};

		};

		// function pointer
		template <typename R, typename... Args>
		struct function_traits<R(*)(Args...)> : function_traits<R(Args...)> {
			using type = R(*)(Args...);
		};

		//const, volatile specialization member function
		template <typename R, typename C, typename... Args>
		struct function_traits<R(C::*)(Args...)> : function_traits<R(Args...)> {
			using class_type = C;
			using type = R(C::*)(Args...);
		};

		//std::function
		template <typename R, typename... Args>
		struct function_traits<std::function<R(Args...)>> : function_traits<R(Args...)> {
			using type = std::function<R(Args...)>;
		};

		//function object / functor / lambda
		template <typename F>
		struct function_traits : function_traits<decltype(&F::operator())> {
			using type = decltype(&F::operator());
		};

	}

	//ref: https://zhuanlan.zhihu.com/p/24651043
	//ref: https://www.zhihu.com/question/56399162
	//alternative: TCPL Ch. 3 dcl
	namespace sn_type_descriptor
	{
		using std::string;

		template <class ...Args>
		struct type_descriptor {
			static string descript() {
				return "[unknown type, maybe \"" + typeid(T).name() + "\"]";
			}
		};

		template <>
		struct type_descriptor<> {
			static string descript() {
				return "void";
			}
		};

#define sn_type_descriptor_generator(type_name) \
	template <> \
	struct type_descriptor<type_name> { \
		static string descript() { \
			return #type_name; \
		} \
	};

		sn_type_descriptor_generator(long long int)

		template <>
		struct type_descriptor<void> {
			static string descript() {
				return "void";
			}
		};

		template <>
		struct type_descriptor<char> {
			static string descript() {
				return "char";
			}
		};

		template <>
		struct type_descriptor<wchar_t> {
			static string descript() {
				return "wchar_t";
			}
		};

		template <>
		struct type_descriptor<bool> {
			static string descript() {
				return "bool";
			}
		};

		template <>
		struct type_descriptor<short> {
			static string descript() {
				return "short";
			}
		};

		template <>
		struct type_descriptor<int> {
			static string descript() {
				return "int";
			}
		};

		template <>
		struct type_descriptor<long> {
			static string descript() {
				return "long";
			}
		};

		template <>
		struct type_descriptor<float> {
			static string descript() {
				return "float";
			}
		};

		template <>
		struct type_descriptor<double> {
			static string descript() {
				return "double";
			}
		};

		template<class T1, class ...Args>
		struct type_descriptor<T1, Args...> {
			static string descript() {
				return type_descriptor<T1>::descript() +", " + type_descriptor<Args...>::descript();
			}
		};

		template<class F>
		struct type_descriptor<F()> {
			static string descript() {
				return "function () -> " + type_descriptor<F>::descript();
			}
		};

		template<class F, class ...Args>
		struct type_descriptor<F(Args...)> {
			static string descript() {
				return "function (" + type_descriptor<Args...>::descript()  +") -> " + type_descriptor<F>::descript();
			}
		};

		template<class T>
		struct type_descriptor<T&> {
			static string descript() {
				return "reference to " + type_descriptor<T>::descript();
			}
		};

		template<class T>
		struct type_descriptor<T*> {
			static string descript() {
				return "pointer to " + type_descriptor<T>::descript();
			}
		};

		template<class R>
		struct type_descriptor<R(*)(void)> {
			static string descript() {
				return "function pointer to function () ->" + type_descriptor<T>::descript();
			}
		};

		template<class R, class ...Args>
		struct type_descriptor<R(*)(Args...)> {
			static string descript() {
				return "function pointer to " + "function (" + type_descriptor<Args...>::descript() + ") -> " + type_descriptor<F>::descript(); 
			}
		};

		template<class T>
		struct type_descriptor<T* const> {  //only match to top-level const, so const T* cannot be match, only T* const
			static string descript() {
				return "const pointer to " + type_descriptor<T>::descript();
			}
		};

		template<class T>
		struct type_descriptor<T* volatile> {
			static string descript() {
				return "volatile pointer to " + type_descriptor<T>::descript();
			}
		};

		template<class T>
		struct type_descriptor<T const> {
			static string descript() {
				return "const " + type_descriptor<T>::descript();
			}
		};

		template<class T>
		struct type_descriptor<T volatile> {
			static string descript() {
				return "volatile " + type_descriptor<T>::descript();
			}
		};

		template<class T>
		struct type_descriptor<T[]> {
			static string descript() {
				return "array of " + type_descriptor<T>::descript();
			}
		};

		template<class T, size_t N>
		struct type_descriptor<T[N]> {
			static string descript() {
				return "array (size " + to_string(N) + ") of " + type_descriptor<T>::descript();
			}
		};

		template<class T, size_t N>
		struct type_descriptor<const T[N]> { //otherwise, top-level const and top-level array will both match
			static string descript() {
				return "array (size " + to_string(N) + ") of const " + type_descriptor<T>::descript();
			}
		};

		template<class T, size_t N>
		struct type_descriptor<volatile T[N]> {
			static string descript() {
				return "array (size " + to_string(N) + ") of volatile " + type_descriptor<T>::descript();
			}
		};

	}

	namespace sn_overload {
		//ref: https://www.zhihu.com/question/37202431
		//ref: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0051r2.pdf
		template <typename F, typename...Args>
		struct func_overload_impl : F, func_overload_impl<Args...> {
			using F::operator();
			using func_overload_impl<Args...>::operator();
			func_overload_impl(F func_, Args... args_) : F(func_), func_overload_impl<Args...>(args_...) {}
		};

		template <typename F>
		struct func_overload_impl<F> : F {
			using F::operator();
			func_overload_impl(F func_) : F(func_) {}
		};

		template <typename ...Args>
		decltype(auto) make_overload_func(Args... args) {
			return func_overload_impl<Args...>{args...};
		}


	}

	namespace sn_tuple_assist {
		//ref : http://stackoverflow.com/a/6245777/273767
		template<class Ch, class Tr, class Tuple, std::size_t... Is>
		void print_tuple_impl(std::basic_ostream<Ch, Tr>& os,
			const Tuple & t,
			std::index_sequence<Is...>)
		{
			using swallow = int[]; // guarantees left to right order
			(void)swallow {
				0, (void(os << (Is == 0 ? "" : ", ") << std::get<Is>(t)), 0)...
			};
		}

		template<class Ch, class Tr, class... Args>
		decltype(auto) operator<<(std::basic_ostream<Ch, Tr>& os,
			const std::tuple<Args...>& t)
		{
			os << "(";
			print_tuple_impl(os, t, std::index_sequence_for<Args...>{}); //can use fold expression easily, ref: http://en.cppreference.com/w/cpp/language/fold
			return os << ")";
		}

		template<typename F, typename T, std::size_t... I>
		decltype(auto) invoke_impl(F&& func, T&& t, std::index_sequence<I...>)
		{
			return func(std::get<I>(std::forward<T>(t))...);
		}

		template<typename F, typename T>
		decltype(auto) invoke_tuple(F&& func, T&& t)
		{
			constexpr auto size = std::tuple_size<typename std::decay<T>::type>::value;
			return invoke_impl(std::forward<F>(func), std::forward<T>(t), std::make_index_sequence<size>{});
		}
	}

	namespace sn_functional_base {
		template <typename T>
		class less_than {
			
			bool operator<=(const T& rhs) {
				return static_cast<T*>(this)->operator<(rhs) || static_cast<T*>(this)->operator=(rhs);
			}

			bool operator>(const T& rhs) {
				return !static_cast<T*>(this)->operator<(rhs);
			}

			bool operator>=(const T& rhs) {
				return !static_cast<T*>(this)->operator<(rhs) || static_cast<T*>(this)->operator=(rhs);
			}

			bool operator!=(const T& rhs) {
				return !static_cast<T*>(this)->operator=(rhs);
			}

		};

		class noncopyable {
		private:
			noncopyable(const noncopyable&) = delete;
			noncopyable& operator=(const noncopyable&) = delete;
		};

		class nonmoveable {
		private:
			nonmoveable(nonmoveable&&) = delete;
			nonmoveable& operator=(nonmoveable&&) = delete;
		};

		class noncopymoveable : public noncopyable, public nonmoveable {};


	}

	//ref: https://github.com/akemimadoka/NatsuLib/blob/master/NatsuLib/natConcepts.h
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

	namespace sn_invoke {
		template <typename F, typename ...Args>
		auto invoke(F&& fn, Args&&... args) noexcept(noexcept(std::forward<F>(fn)(std::forward<Args>(args)...))) {
			return std::forward<F>(fn)(std::forward<Args>(args)...);
		}

		template <typename R, typename C, typename ...Args>
		auto invoke(R(C::*ptr_fn)(Args...), C* p, Args&&... args) noexcept(noexcept(p->ptr_fn(std::forward<Args>(args)...))) {
			return p->*ptr_fn(std::forward<Args>(args)...);
		}

		template <typename R, typename C, typename ...Args>
		auto invoke(R(C::*ptr_fn)(Args...), C&& c, Args&&... args) noexcept(noexcept(std::forward<C>(c).ptr_fn(std::forward<Args>(args)...))) {
				return std::forward<C>(c).*ptr_fn(std::forward<Args>(args)...);
		}

#define SN_INVOKE_GEN(SUFFIX) \
		template <typename R, typename C, typename ...Args> \
		auto invoke(R(C::*ptr_fn)(Args...) SUFFIX, C* p, Args&&... args) noexcept(noexcept(p->ptr_fn(std::forward<Args>(args)...))) { \
			return p->*ptr_fn(std::forward<Args>(args)...); \
		} \
		template <typename R, typename C, typename ...Args> \
		auto invoke(R(C::*ptr_fn)(Args...) SUFFIX, C&& c, Args&&... args) noexcept(noexcept(std::forward<C>(c).ptr_fn(std::forward<Args>(args)...))) { \
			return std::forward<C>(c).*ptr_fn(std::forward<Args>(args)...); \
		} \


		//For p::f() & and p::f() &&
		SN_INVOKE_GEN(&)
		SN_INVOKE_GEN(&&)
		SN_INVOKE_GEN(const)
		SN_INVOKE_GEN(const &)
		SN_INVOKE_GEN(const &&)
		SN_INVOKE_GEN(volatile)
		SN_INVOKE_GEN(volatile &)
		SN_INVOKE_GEN(volatile &&)
		SN_INVOKE_GEN(const volatile)
		SN_INVOKE_GEN(const volatile &)
		SN_INVOKE_GEN(const volatile &&)
	}

	namespace sn_cast {
		template <typename T, typename U, typename = U>
		struct static_dynamic_cast_impl{
			static U impl(T&& t) {
				return dynamic_cast<U>(static_cast<T&&>(t));
			}
		};
		template <typename T, typename U>
		struct static_dynamic_cast_impl<T, U, decltype(static_cast<U>(std::declval<T>()))> {
			static U impl(T&& t) {
				return static_cast<U>(static_cast<T&&>(t));
			}
		};

		template <typename T, typename U>
		constexpr U static_dynamic_cast(T&& t) {
			return static_dynamic_cast_impl<T, U>::impl(t);
		}

	}

}








#endif