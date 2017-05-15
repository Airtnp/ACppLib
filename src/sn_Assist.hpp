#ifndef SN_ASSIST_H
#define SN_ASSIST_H

#include "sn_CommonHeader.h"

namespace sn_Assist {

	namespace sn_demangle {
		template <typename T>
		std::string demangle_type() {
			using TR = typename std::remove_reference<T>::type;
			std::unique_ptr<char, void(*)(void*)> own;
				(
			#ifndef _MSC_VER
				abi::__cxa_demangle(typeid(TR).name(), nullptr, nullptr, nullptr),
			#else
				nullptr,
			#endif
				std::free
				);
			std::string r = own != nullptr ? own.get() : typeid(TR).name();
			if (std::is_const<TR>::value)
				r += " const";
			if (std::is_volatile<TR>::value)
				r += " volatile";
			if (std::is_lvalue_reference<TR>::value)
				r += "&";
			if (std::is_rvalue_reference<TR>::value)
				r += "&&";
			return r;
		}
	}

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

		template <class T, class V /* = std::void_t<>*/, template<class...> class Op, class... Args>
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
		constexpr static const bool sn_is_detected_v = sn_is_detected<Op, Args...>::value;

		template <template<class...> class Op, class... Args>
		using sn_detected_v = typename sn_detector<sn_nonesuch, void, Op, Args...>::type;


		/*
		Usage:
		template<class T>
		using has_member_r = decltype(&T::r)

		struct Type {
			int r;
		}

		sn_is_detected_v<has_member_r, Type>
		*/

		template <bool>
		struct statement_if_impl {
			template <typename F, typename P, typename ...Args>
			static decltype(auto) run(F&& f, P&& p, Args&&... args) {
				return std::forward<F>(f)(std::forward<Args>(args)...);
			}
		};

		template <>
		struct statement_if_impl<false> {
			template <typename F, typename P, typename ...Args>
			static decltype(auto) run(F&& f, P&& p, Args&&... args) {
				return std::forward<P>(p)(std::forward<Args>(args)...);
			}
		};

		template <bool s, typename F, typename P, typename ...Args>
		decltype(auto) statement_if(F&& f, P&& p, Args&&... args) {
			return statement_if_impl<s>::run(
				std::forward<F>(f), std::forward<P>(p), std::forward<Args>(args)...
			);
		}

	}

	namespace sn_has_member {

		// Old-style : see sn_TypeTraits

		//std::experimental::is_detected
		//__if_exists in VS
		//Usage: SN_HAS_MEMBER(a); bool has_member_a = SN_HAS_MEMBER_VALUE<T>(a);

#define SN_HAS_MEMBER(member_name) \
	template <typename T, typename V = std::void_t<>> \
	struct sn_has_member_##member_name : std::false_type {}; \
	template <typename T> \
	struct sn_has_member_##member_name<T, std::void_t<decltype(T::##member_name)>> : std::true_type {};

#define SN_HAS_MEMBER_VALUE(Type, member_name) \
	sn_has_member_##member_name<Type>::value


#define SN_HAS_MEMBER_FUNCTION(func_name) \
	template <typename C, typename R, typename V = std::void_t<>, typename... Args> \
	struct sn_has_member_function_##func_name : std::false_type {}; \
	template <typename C, typename R, typename... Args> \
	struct sn_has_member_function_##func_name<C, R(Args...), \
		std::enable_if_t< \
			std::is_same< \
				R, decltype(std::declval<C>().##func_name(std::declval<Args>()...)) \
			>::value, R \
		>, Args...> : std::true_type {};

#define SN_HAS_MEMBER_FUNCTION_VALUE(cls, func_name, ret, ...) \
	sn_has_member_function_##func_name<cls, ret(__VA_ARGS__)>::value;

		/*
		#define SN_HAS_MEMBER_FUNCTION(func_name) \
		template <typename C, typename R, typename V = std::void_t<>, typename... Args> \
		struct sn_has_member_function_##func_name : std::false_type {}; \
		template <typename C, typename R, typename... Args> \
		struct sn_has_member_function_##func_name<C, R(Args...), \
		std::void_t< \
		decltype(std::declval<C>().##func_name(std::declval<Args>()...)) \
		>, \
		Args...> : std::true_type {};
		*/

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

		/*
		Old-style

		#define CREATE_MEMBER_DETECTOR(X)                                                   	\
			template<typename T> class Detect_##X {                                             \
				struct Fallback { int X; };                                                     \
				struct Derived : T, Fallback { };                                               \
																								\
				template<typename U, U> struct Check;                                           \
																								\
				typedef char ArrayOfOne[1];                                                     \
				typedef char ArrayOfTwo[2];                                                     \
																								\
				template<typename U> static ArrayOfOne & func(Check<int Fallback::*, &U::X> *); \
				template<typename U> static ArrayOfTwo & func(...);                             \
			public:                                                                           	\
				typedef Detect_##X type;                                                        \
				enum { value = sizeof(func<Derived>(0)) == 2 };                                 \
			};
		
		*/


	}

	// This cannot support template function | generic lambda
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
			using function_type = R(Args...);
			using stl_function_type = std::function<function_type>;

			template <size_t I>
			struct args {
				using type = typename std::tuple_element<I, std::tuple<Args...>>::type;
			};

		};

		// function pointer
		template <typename R, typename... Args>
		struct function_traits<R(*)(Args...)> : public function_traits<R(Args...)> {
			using type = R(*)(Args...);
		};

		//const, volatile specialization member function
		template <typename R, typename C, typename... Args>
		struct function_traits<R(C::*)(Args...)> : public function_traits<R(Args...)> {
			using class_type = C;
			using type = R(C::*)(Args...);
		};

		template <typename R, typename C, typename... Args>
		struct function_traits<R(C::*)(Args...) const> : public function_traits<R(Args...)> {
			using class_type = C;
			using type = R(C::*)(Args...);
		};

		//std::function
		template <typename R, typename... Args>
		struct function_traits<std::function<R(Args...)>> : public function_traits<R(Args...)> {
			using type = std::function<R(Args...)>;
		};

		//function object / functor / lambda
		template <typename F>
		struct function_traits : public function_traits<decltype(&F::operator())> {
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
				std::string tmp = "[unknown type, maybe \"";
				std::initializer_list<int>{(tmp = tmp + sn_demangle::demangle_type<Args>() + " ", 0)...};
				tmp += "\"]";
				return tmp;
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
				return "lvalue reference to " + type_descriptor<T>::descript();
			}
		};

		template<class T>
		struct type_descriptor<T&&> {
			static string descript() {
				return "rvalue reference to " + type_descriptor<T>::descript();
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
				return "function pointer to (function () -> " + type_descriptor<R>::descript() + ")";
			}
		};

		template<class R, class ...Args>
		struct type_descriptor<R(*)(Args...)> {
			static string descript() {
				return std::string("function pointer to ") + "(function (" + type_descriptor<Args...>::descript() + ") -> " + type_descriptor<R>::descript() + ")";
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
		using std::to_string;
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
		
		// overload even can be used to detect member
		/* auto o = make_overload_func(
				[](auto&& c, auto&&... args) -> decltype(c.func_name(std::forward<Args>(args)...)) {
					return c.func_name(std::forward<Args>(args)...);
				},
				[](auto&& c, auto&&... args) {
					// do nothing
				}
			);
		*/

		// ref: https://www.zhihu.com/question/37202431
		// ref: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0051r2.pdf
		// In C++1z, you can straightly
		// class F : P... 		
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

		class nondirectconstructable {
		protected:
			void* operator new(std::size_t size) {
				return ::operator new(size);
			}
			void operator delete(void* ptr) {
				::operator delete(ptr);
			}
		};

		// default_constructor
		template <bool dc>
		struct enable_default_constructor {};

		template <>
		struct enable_default_constructor<true> {
			using enable_type = enable_default_constructor;
			enable_default_constructor() = default;
			constexpr enable_default_constructor(void*) {}
		};
		template <>
		struct enable_default_constructor<false> {
			using enable_type = enable_default_constructor;
			enable_default_constructor() = delete;
			constexpr enable_default_constructor(void*) {}
		};
	}

	// ref: https://github.com/akemimadoka/NatsuLib/blob/master/NatsuLib/natConcepts.h
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

	// TODO: ref: YSLib/YBase/include/ystdex/cast.hpp
	namespace sn_cast {
		template <typename T, typename U, typename = U>
		struct static_dynamic_cast_impl{
			static U impl(T& t) {
				return dynamic_cast<U>(std::forward<T>(t));
			}
		};
		template <typename T, typename U>
		struct static_dynamic_cast_impl<T, U, decltype(static_cast<U>(std::declval<T>()))> {
			static U impl(T& t) {
				return static_cast<U>(std::forward<T>(t));
			}
		};

		template <typename T, typename U>
		constexpr U static_dynamic_cast(T&& t) {
			return static_dynamic_cast_impl<T, U>::impl(t);
		}

	}

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








#endif