#ifndef SN_PM_H
#define SN_PM_H

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"
#include "sn_TypeLisp.hpp"
#include "sn_Type.hpp"
#include "sn_Function.hpp"

// Yes, it failed in VS2015! Simply ICE!
// Yes, it failed in Clang3.9! Illegal Instruction!
// Yes, it succeeded in gcc6.3!
// TODO: runtime match ref : https://github.com/solodon4/Mach7/tree/master/code
// TODO: runtime match using structure binding. inspect(s) { [..., ...] = s => ... };
namespace sn_PM {
	using sn_TypeLisp::TypeList;
	using sn_Assist::sn_require::Require;
	using sn_Assist::sn_function_traits::function_traits;
	using sn_Type::variant::Variant;
	using sn_Type::any::Any;
	using sn_Function::function::Func;
	using sn_Function::make_func;
	using sn_Function::make_curry;
	using sn_Function::make_single_curry;
	using sn_Function::make_multi_curry;

	namespace pattern {
		template <typename ...Args>
		struct SwitchFunc;
	}

	namespace def {

		using pattern::SwitchFunc;

		// -------- Basic Sugar --------- //
		// For (Int, Char) => Type<int> >= Type<char>
		template <typename T>
		struct Type {
			using type = T;
		};

		template <typename T, typename U>
		struct Type<T(U)> {
			typedef T type(U);
			using return_type = T;
			using parameter_type = U;
		};

		// substitue [int]
		template <typename T>
		struct List : public Type<T[]> {};

		// substitute ( -> )
		template <typename T, typename U>
		struct MapType {};

		template <typename T, typename U>
		struct MapType<Type<T>, Type<U>> : Type<MapType<Type<T>, Type<U>>> {};

		// A |= Type<int> >= Type<int>
		template <typename T, typename U>
		auto operator>=(Type<T>, Type<U>) {
			return Type<U(T)>{};
		}

		// int -> ( int -> float ) or int -> int -> float
		template <typename T, typename U>
		auto operator>(Type<T>, Type<U>) {
			return Type<MapType<T, U>>{};
		}

		namespace lib {
			struct Eq {
				template <typename T>
				constexpr auto require(T a, T b) -> decltype(a == b) { return a == b; }
			};

			struct Ne {
				template <typename T>
				constexpr auto require(T a, T b) -> decltype(a != b) { return a != b; }
			};

			struct Lt {
				template <typename T>
				constexpr auto require(T a, T b) -> decltype(a < b) { return a < b; }
			};

			struct Gt {
				template <typename T>
				constexpr auto require(T a, T b) -> decltype(a < b) { return a < b; }
			};

			struct Le {
				template <typename T>
				constexpr auto require(T a, T b) -> decltype(a < b) { return a < b; }
			};

			struct Ge {
				template <typename T>
				constexpr auto require(T a, T b) -> decltype(a < b) { return a < b; }
			};

			// Ord<T>
			template <typename T>
			using Ord = TypeList<Ge(T, T), Le(T, T), Gt(T, T), Gt(T, T)>;

			// Eqn<T>
			template <typename T>
			using Eqn = TypeList<Eq(T, T), Ne(T, T)>;
		}

		// template <typename T>
		// auto p = Func |= ( ( RClass<Eq(T, T)> | RClass<Ne(T, T)> ) >>= Type<T> >= Type<int> )
		template <typename ...Args>
		struct RClass {};

		template <typename Arg, typename ...Args>
		struct RClass<Arg, Args...> {
			using concept = TypeList<Arg, Args...>;
			constexpr static const bool value = Require<Arg>::value && RClass<Args...>::value;
		};

		template <typename ...Args>
		struct RClass<TypeList<Args...>> {
			using concept = TypeList<Args...>;
			constexpr static const bool value = RClass<Args...>::value;
		};

		template <>
		struct RClass<> {
			using concept = TypeList<>;
			constexpr static const bool value = true;
		};

		template <typename T, typename U>
		struct CombineRClass {};

		template <typename ...Args1, typename ...Args2>
		struct CombineRClass<RClass<Args1...>, RClass<Args2...>> : RClass<Args1..., Args2...> {};

		template <typename ...Args1, typename ...Args2>
		auto operator|(RClass<Args1...>, RClass<Args2...>) {
			return RClass<Args1..., Args2...>{};
		}

		// -------- Restrict Function --------- //
		template <typename T>
		struct ExtractType {};

		template <typename T>
		struct ExtractType<Type<T>> {
			using type_list = TypeList<T>;
			static constexpr const std::size_t type_length = sn_TypeLisp::TypeLength_v<type_list>;
		};

		template <typename T, typename U>
		struct ExtractType<Type<U(T)>> {
			using type_list = sn_TypeLisp::TypeAppend_t<
				typename ExtractType<Type<T>>::type_list,
				typename ExtractType<Type<U>>::type_list
			>;
			static constexpr const std::size_t type_length = sn_TypeLisp::TypeLength_v<type_list>;
		};

		// function will decay
		template <typename T, typename U>
		struct ExtractType<Type<U(*)(T)>> {
			using type_list = sn_TypeLisp::TypeAppend_t<
				typename ExtractType<Type<T>>::type_list,
				typename ExtractType<Type<U>>::type_list
			>;
			static constexpr const std::size_t type_length = sn_TypeLisp::TypeLength_v<type_list>;
		};

		template <typename T, typename U>
		struct ExtractTypeList {};

		template <typename U, typename ...Args>
		struct ExtractTypeList<U, TypeList<Args...>> {
			typedef U(*func_type)(Args...);
			using type_list = TypeList<func_type>;
		};


		template <typename T, typename U>
		struct ExtractType<Type<MapType<T, U>>> {
			typedef U(*func_type)(typename ExtractType<Type<T>>::type_list);
			using type_list = typename ExtractTypeList<U, typename ExtractType<Type<T>>::type_list>::type_list;
			static constexpr const std::size_t type_length = sn_TypeLisp::TypeLength_v<type_list>;
		};

		template <typename T>
		struct ConstructResultTypeList {};

		template <typename Arg, typename ...Args>
		struct ConstructResultTypeList<TypeList<Arg, Args...>> {
			using func_type = Func<Arg(typename ConstructResultTypeList<TypeList<Args...>>::func_type)>;
			// or std::add_pointer_t<typename ConstructResultTypeList<TypeList<Args...>>::func_type>
		};

		template <typename Arg1, typename Arg2>
		struct ConstructResultTypeList<TypeList<Arg1, Arg2>> {
			using func_type = Func<Arg1(Arg2)>;
		};

		template <typename Arg>
		struct ConstructResultTypeList<TypeList<Arg>> {
			using func_type = Arg;
		};

		template <typename R, typename ...Args>
		struct ConstructResultTypeList<TypeList<R, TypeList<Args...>>> {
			using func_type = Func<R(Args...)>;
		};

		template <typename T>
		struct ConstructResultType {};

		template <typename Arg, typename ...Args>
		struct ConstructResultType<TypeList<Arg, Args...>> {
			using result_type_list = sn_TypeLisp::TypeAppend_t<TypeList<typename ConstructResultTypeList<Arg>::func_type>, typename ConstructResultType<TypeList<Args...>>::result_type_list>;
		};

		template <>
		struct ConstructResultType<TypeList<>> {
			using result_type_list = TypeList<>;
		};

		template <typename T>
		struct ConstructVariantType {};

		template <typename ...Args>
		struct ConstructVariantType<TypeList<Args...>> {
			using variant_type = Variant<Args...>;
		};

		template <typename T, typename U>
		struct ConstructFuncType {};

		template <typename Arg1, typename ...Args1, typename Arg2, typename ...Args2>
		struct ConstructFuncType<TypeList<Arg1, Args1...>, TypeList<Arg2, Args2...>> {
			using func_type_list = sn_TypeLisp::TypeAppend_t<
				TypeList<Func<typename ConstructResultTypeList<TypeList<Arg1, Arg2>>::func_type>>,
				typename ConstructFuncType<
				TypeList<Args1...>,
				TypeList<Args2...>
				>::func_type_list
			>;
			using variant_type = typename ConstructVariantType<func_type_list>::variant_type;
		};

		template <>
		struct ConstructFuncType<TypeList<>, TypeList<>> {
			using func_type_list = TypeList<>;
		};

		template <typename T, typename U>
		struct VariantFuncList {};

		template <typename Arg, typename ...Args, typename ...Args2>
		struct VariantFuncList<TypeList<Arg, Args...>, TypeList<Args2...>> {
			using current_param_list = sn_TypeLisp::TypeDropOne_t<
				typename VariantFuncList<
				TypeList<Args...>,
				TypeList<Args2...>
				>::current_param_list
			>;
			using param_list = sn_TypeLisp::TypeAppend_t<
				TypeList<current_param_list>,
				typename VariantFuncList<
				TypeList<Args...>,
				TypeList<Args2...>
				>::param_list
			>;
			using current_result_list = sn_TypeLisp::TypeAppend_t<
				typename VariantFuncList<
				TypeList<Args...>,
				TypeList<Args2...>
				>::current_result_list,
				Arg
			>;
			using result_list = sn_TypeLisp::TypeAppend_t<
				TypeList<current_result_list>,
				typename VariantFuncList<
				TypeList<Args...>,
				TypeList<Args2...>
				>::result_list
			>;
			using result_type_list = typename ConstructResultType<result_list>::result_type_list;
			using func_type_list = typename ConstructFuncType<result_type_list, param_list>::func_type_list;
			using variant_type = typename ConstructFuncType<result_type_list, param_list>::variant_type;
		};

		template <typename R, typename ...Args2>
		struct VariantFuncList<TypeList<R>, TypeList<Args2...>> {
			using current_param_list = sn_TypeLisp::TypeDropOne_t<TypeList<Args2...>>;
			using param_list = TypeList<current_param_list>;
			using current_result_list = TypeList<R>;
			using result_list = TypeList<current_result_list>;
			using result_type_list = typename ConstructResultType<result_list>::result_type_list;
		};

		struct FuncTypeWrapperHead {};

		template <typename ...Args>
		struct FuncTypeWrapper {};

		// TODO: add operator() in variant
		// substitue >>=
		template <typename ...Args, typename U>
		struct FuncTypeWrapper <RClass<Args...>, Type<U>> : FuncTypeWrapperHead {
			using rclass = RClass<Args...>;
			using require_list = typename rclass::concept;
			constexpr static const bool rvalue = rclass::value;
			static_assert(rvalue, "Classes not satisfied.");

			using type = U;
			using type_list = typename ExtractType<Type<U>>::type_list;
			using func_type_list = typename VariantFuncList<type_list, type_list>::func_type_list;
			using variant_type = typename VariantFuncList<type_list, type_list>::variant_type;

			variant_type var;
			Any any;
			template <typename T>
			FuncTypeWrapper& operator=(T&& func) {
				try {
					var = make_func(std::forward<T>(func));
				}
				catch (...) {
					throw std::runtime_error("Not matched");
				}
			}
			template <typename ...PArgs>
			FuncTypeWrapper& operator=(SwitchFunc<PArgs...> s) {
				any = s;
			}
			template <typename C, typename T>
			void assign(C* obj, T&& func) {
				try {
					var = make_func(obj, std::forward<T>(func));
				}
				catch (...) {
					throw std::runtime_error("Not matched");
				}
			}
			template <typename F, typename ...TArgs>
			auto operator()(F&& f, TArgs&&... args) {
				if (var.empty())
					throw std::runtime_error("Not initialized.");
				auto func = var.template get<decltype(make_func(f))>();
				return func(std::forward<TArgs>(args)...);
			}
			template <typename ...PArgs, typename ...TArgs>
			auto operator()(SwitchFunc<PArgs...>, TArgs&&... args) {
				if (any.is_null())
					throw std::runtime_error("Not initialized.");
				auto func = any.template any_cast<SwitchFunc<PArgs...>>();
				return func(std::forward<TArgs>(args)...);
			}
			template <typename C, typename F, typename ...TArgs>
			auto operator()(C* obj, F&& f, TArgs&&... args) {
				if (var.empty())
					throw std::runtime_error("Not initialized.");
				auto func = var.template get<decltype(make_func(obj, f))>();
				return func(std::forward<TArgs>(args)...);
			}
		};

		template <typename U>
		struct FuncTypeWrapper<Type<U>> : FuncTypeWrapperHead {
			using rclass = RClass<>;
			using require_list = typename rclass::concept;
			constexpr static const bool rvalue = true;

			using type = U;
			using type_list = typename ExtractType<Type<U>>::type_list;
			using func_type_list = typename VariantFuncList<type_list, type_list>::func_type_list;
			using variant_type = typename VariantFuncList<type_list, type_list>::variant_type;

			variant_type var;
			Any any;
			template <typename T>
			FuncTypeWrapper& operator=(T&& func) {
				try {
					var = make_func(std::forward<T>(func));
				}
				catch (...) {
					throw std::runtime_error("Not matched");
				}
			}
			template <typename ...PArgs>
			FuncTypeWrapper& operator=(SwitchFunc<PArgs...> s) {
				any = s;
			}
			template <typename C, typename T>
			void assign(C* obj, T&& func) {
				try {
					var = make_func(obj, std::forward<T>(func));
				}
				catch (...) {
					throw std::runtime_error("Not matched");
				}
			}
			template <typename F, typename ...TArgs>
			auto operator()(F&& f, TArgs&&... args) {
				if (var.empty())
					throw std::runtime_error("Not initialized.");
				auto func = var.template get<decltype(make_func(f))>();
				return func(std::forward<TArgs>(args)...);
			}
			template <typename ...PArgs, typename ...TArgs>
			auto operator()(SwitchFunc<PArgs...>, TArgs&&... args) {
				if (any.is_null())
					throw std::runtime_error("Not initialized.");
				auto func = any.template any_cast<SwitchFunc<PArgs...>>();
				return func(std::forward<TArgs>(args)...);
			}
			template <typename C, typename F, typename ...TArgs>
			auto operator()(C* obj, F&& f, TArgs&&... args) {
				if (var.empty())
					throw std::runtime_error("Not initialized.");
				auto func = var.template get<decltype(make_func(obj, f))>();
				return func(std::forward<TArgs>(args)...);
			}
		};

		template <typename ...Args, typename U>
		auto operator>>=(RClass<Args...>, Type<U>) {
			return FuncTypeWrapper<RClass<Args...>, Type<U>>{};
		}

		// FuncTypeWrapperHead |= FuncTypeWrapper<....>
		template <typename ...Args>
		auto operator|=(FuncTypeWrapperHead, FuncTypeWrapper<Args...>) {
			return FuncTypeWrapper<Args...>{};
		}

		template <typename U>
		auto operator|=(FuncTypeWrapperHead, Type<U>) {
			return FuncTypeWrapper<Type<U>>{};
		}

	}

	// Separate this into single file
	namespace pattern {
		using def::VariantFuncList;
		using def::Type;
		
		template <typename L>
		struct MatchTypeListToTuple {};

		template <typename ...Args>
		struct MatchTypeListToTuple<TypeList<Args...>> {
			using tuple_type = std::tuple<Args...>;
		};

		template <typename F>
		struct MatchFunc {};

		template <typename R, typename ...Args>
		struct MatchFunc<Func<R(Args...)>> {
			using result_type = R;
			using argument_type = TypeList<Args...>;
		};

		template <typename ...Args>
		struct Switch : public Switch<TypeList<Args...>> {
			using avail_type_list = TypeList<Args...>;
		};

		template <typename ...Args>
		struct Switch<Type<Args>...> : public Switch<TypeList<Args...>> {
			using avail_type_list = TypeList<Args...>;
		};

		template <typename ...Args>
		struct Switch<TypeList<Args...>> {
			using avail_type_list = TypeList<Args...>;
		};

		template <typename ...Args>
		struct Case {
			Case() {}
		};

		template <typename ...Args, typename ...PArgs>
		struct Case<Switch<Args...>, PArgs...> {
			using v_list = VariantFuncList<TypeList<Args...>, TypeList<Args...>>;
			constexpr static const std::size_t N = sn_TypeLisp::TypeIndex<typename v_list::param_list, TypeList<PArgs...>>::value;
			static_assert(N != -1, "Failed to find availible params pattern.");
			using R = sn_TypeLisp::TypeAt_t<typename v_list::result_type_list, N>;
			using func_type = Func<R(PArgs...)>;
			Case() {}
			Func<R(PArgs...)> m_func;
			template <typename T, typename V = std::enable_if_t<std::is_same<func_type, decltype(make_func(std::declval<T>()))>::value>>
			Case& operator=(T&& func) {
				m_func = make_func(func);
				return *this;
			}
			template <typename T, typename V = std::enable_if_t<std::is_same<func_type, decltype(make_func(std::declval<T>()))>::value>>
			Case& assign(T&& func) {
				m_func = make_func(func);
				return *this;
			}
			template <typename C, typename T, typename V = std::enable_if_t<std::is_same<func_type, decltype(make_func(std::declval<C*>(), std::declval<T>()))>::value>>
			Case& assign(C* obj, T&& func) {
				m_func = make_func(obj, func);
				return *this;
			}
		};

		template <typename ...Args>
		struct ConstructTupleType {};

		template <typename S, typename ...CArgs, typename ...Cs>
		struct ConstructTupleType<S, Case<CArgs...>, Cs...> {
			using func_type_list = sn_TypeLisp::TypeAppend_t<TypeList<typename Case<S, CArgs...>::func_type>, typename ConstructTupleType<S, Cs...>::func_type_list>;
			using tuple_type = typename MatchTypeListToTuple<func_type_list>::tuple_type;
		};

		template <typename S>
		struct ConstructTupleType<S> {
			using func_type_list = TypeList<>;
		};

		template <typename ...Args, std::size_t ...I>
		auto remove_first_tuple_impl(const std::tuple<Args...>& tp, std::index_sequence<I...>) {
			return std::make_tuple(std::get<I + 1>(tp)...);
		}

		template <typename ...Args>
		auto remove_first_tuple(const std::tuple<Args...>& tp) {
			return remove_first_tuple_impl(tp, std::make_index_sequence<sizeof...(Args)-1>{});
		}

		template <typename C>
		struct InheritFuncObject {};

		template <typename Arg, typename ...Args>
		struct InheritFuncObject<TypeList<Arg, Args...>> {
			using tuple_type = std::tuple<Arg, Args...>;
			using parent_type = InheritFuncObject<TypeList<Args...>>;
			using R = typename MatchFunc<Arg>::result_type;
			using Arg_type = typename MatchFunc<Arg>::argument_type;
			Arg m_func;
			parent_type m_parent;
			InheritFuncObject(tuple_type tp) : m_func(std::get<0>(tp)), m_parent(remove_first_tuple(tp)) {}
			template <typename ...TArgs, typename V = std::enable_if_t<std::is_same<TypeList<TArgs...>, Arg_type>::value>>
			R operator()(TArgs&&... args) {
				return m_func(std::forward<TArgs>(args)...);
			}
			template <typename ...TArgs, typename V = std::enable_if_t<!std::is_same<TypeList<TArgs...>, Arg_type>::value>>
			auto operator()(TArgs&&... args) {
				return m_parent(std::forward<TArgs>(args)...);
			}
		};

		template <typename Arg>
		struct InheritFuncObject<TypeList<Arg>> {
			using tuple_type = std::tuple<Arg>;
			using R = typename MatchFunc<Arg>::result_type;
			using Arg_type = typename MatchFunc<Arg>::argument_type;
			Arg m_func;
			InheritFuncObject(tuple_type tp) : m_func(std::get<0>(tp)) {}
			template <typename ...TArgs, typename V = std::enable_if_t<std::is_same<TypeList<TArgs...>, Arg_type>::value>>
			R operator()(TArgs&&... args) {
				return m_func(std::forward<TArgs>(args)...);
			}
		};


		template <typename ...Args>
		struct SwitchFunc {};

		template <typename ...Args, typename ...Cs>
		struct SwitchFunc<Switch<Args...>, Cs...> {
			using func_type_list = typename ConstructTupleType<Switch<Args...>, Cs...>::func_type_list;
			using tuple_type = typename ConstructTupleType<Switch<Args...>, Cs...>::tuple_type;
			tuple_type m_tuple;
			InheritFuncObject<func_type_list> m_func;
			SwitchFunc(tuple_type tp) : m_tuple(tp), m_func(tp) {}
			template <typename ...PArgs>
			auto operator()(PArgs&&... args) {
				return m_func(std::forward<PArgs>(args)...);
			}
		};

		template <typename ...SArgs, typename ...PArgs>
		auto operator|(Switch<SArgs...>, Case<Switch<SArgs...>, PArgs...> c) {
			return SwitchFunc<Switch<SArgs...>, Case<PArgs...>>(c.m_func);
		}

		template <typename S, typename C, typename ...Cs, typename ...PArgs>
		auto operator|(SwitchFunc<S, C, Cs...> r, Case<S, PArgs...> c) {
			using T = SwitchFunc<S, C, Cs..., Case<PArgs...>>;
			return T(std::tuple_cat(r.m_tuple, std::make_tuple(c.m_func)));
		}


	}

	// Finally wrapper all above into namespace and rename this

	using namespace def::lib;

	template <typename U>
	auto Ty = def::Type<U>{};

	template <typename U>
	auto Ls = def::List<U>{};
	
	template <typename ...Args>
	auto R = def::RClass<Args...>{};

	auto F = def::FuncTypeWrapperHead{};

	template <typename ...Args>
	auto Switch = pattern::Switch<Args...>;

	template <typename ...Args>
	auto S = pattern::Switch<Args...>;

	/* 
	Usage:
		For default function, int foo(char, int) ======> q = &foo; q(&foo, params...);
			int foo2(char) int foo(int) -> &foo2 ======> q = &foo2; q(&foo2, params...)(params2...);  (can only communiate in global/namespace variable...)
		For lambda function/class function A::t  ======> q.assign(&a, &A::t);
			                               &lambda::operator()
			TODO: directly match function object (&C::operator())
			WARN: class member function returning member function was failed. (Variant cannot accept this type)
				  Curry cannot fix this because currying return C::Binder
		For Switch<Args...> | Case<S<Args...>, PArgs...> 
			=====> q_def = ...;
			       q_func = Switch<Args...>
							| Case<S<Args...>, PArgs...>{}.assign(...)
							| ...;
				   q_def = q_func;
				   q_def(q_func, ...);
	*/

	// ref: https://github.com/mutouyun/cpp-pattern-matching/blob/master/match.hpp
	namespace runtime {

	}



}


#endif