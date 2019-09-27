#include "../sn_CommonHeader.h"

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

		struct sn_any_constructible {
			sn_any_constructible(...);
		};

		struct sn_nonesuch {
			sn_nonesuch() = delete;
			~sn_nonesuch() = delete;
			sn_nonesuch(sn_nonesuch const&) = delete;
			void operator= (sn_nonesuch const&) = delete;
		};

		template <class T, class V /* = std::void_t<>*/, template<class...> class Op, class... Args>
		struct sn_detector {
			using value_t = std::false_type;
			using type = T; // Default
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

	/*
		However, struct T { void func() & {} }; cannot use std::declval<T>().func()
	*/
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

		#define CREATE_MEMBER_FUNCTION_DETECTOR(X)                                                   	\
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

		struct yes_type { char m[2]; };
		struct no_type { char m[1]; };
		// consider member/ADL/global
		// When the test is false, call ADL swap. Otherwise, perform a function-pointer based test. Call apply2 by
		// taking the address of swap, which is known to be possible because at least one swap exists.
#define SN_DETECT_X_TRAITS_AND_CALL(X) \
	struct Traits_##X { \
		template <typename T, void (T::*F)(T&)> \
		class yes1 : public sn_Assist::sn_has_member::yes_type {}; \
		template <typename T, void (*F)(T&)> \
		class yes2 : public sn_Assist::sn_has_member::yes_type {}; \
		template <typename T> \
		inline static void apply (T& a, T& b) { \
			apply1(a, b, test(&a)); \
		} \
	private: \
		template <typename T> \
		static yes1<T, &T::##X>* test(T*) { return 0; } \
		template <typename T> \
		static yes2<T, &T::##X>* test(T*) { return 0; } \
		static no_type* test(void*) { return 0; } \
		\
		template <typename T> \
		inline static void apply1(T& a, T& b, no_type*) { \
			X##(a, b); \
		} \
		template <typename T> \
		inline static void apply1(T& a, T& b, yes_type*) { \
			apply2(a, b, &T::##X); \
		} \
		template <typename T> \
		inline static void apply2(T& a, T& b, void (*)(T&, T&)) { \
			T::##X##(a, b); \
		} \
		template <typename T, typename BASE> \
		inline static void apply2(T& a, T& b, void (BASE::*)(BASE&)) { \
			a.##X##(b); \
		} \
		template <typename T> \
		inline static void apply2(T& a, T& b, ...) { \
			X##(a, b); \
		} \
	}; \

	}
}