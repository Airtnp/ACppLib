#ifndef SN_PM_H
#define SN_PM_H

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"
#include "sn_TypeLisp.hpp"
#include "sn_Type.hpp"

namespace sn_PM {
	using sn_TypeLisp::TypeList;
	using sn_Assist::sn_require::Require;
	using sn_Assist::sn_function_traits::function_traits;
	using sn_Type::variant::Variant;

	namespace def {

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
			typedef Arg(*func_type)(typename ConstructResultTypeList<TypeList<Args...>>::func_type);
			// or std::add_pointer_t<typename ConstructResultTypeList<TypeList<Args...>>::func_type>
		};

		template <typename Arg1, typename Arg2>
		struct ConstructResultTypeList<TypeList<Arg1, Arg2>> {
			typedef Arg1(*func_type)(Arg2);
		};

		template <typename Arg>
		struct ConstructResultTypeList<TypeList<Arg>> {
			using func_type = Arg;
		};

		template <typename R, typename ...Args>
		struct ConstructResultTypeList<TypeList<R, TypeList<Args...>>> {
			typedef R func_type(Args...);
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
				TypeList<std::add_pointer_t<typename ConstructResultTypeList<TypeList<Arg1, Arg2>>::func_type>>,
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

		// substitue >>=
		template <typename ...Args, typename U>
		struct FuncTypeWrapper <RClass<Args...>, Type<U>> : FuncTypeWrapperHead {
			using rclass = RClass<Args...>;
			using require_list = typename rclass::concept;
			constexpr static const bool rvalue = rclass::value;
			static_assert(rvalue, "Classes not satisfied.");

			using type = U;
			using type_list = typename ExtractType<Type<U>>::type_list;
			using variant_type = typename VariantFuncList<type_list, type_list>::variant_type;

			variant_type var;
			template <typename T>
			FuncTypeWrapper& operator=(T func) {
				var = func;
			}
			template <typename F, typename ...TArgs>
			auto operator()(F&& f, TArgs&&... args) {
				if (var.empty())
					throw std::runtime_error("Not initialized.");
				auto func = var.template get<F>();
				return func(args...);
			}
		};

		template <typename U>
		struct FuncTypeWrapper<Type<U>> : FuncTypeWrapperHead {
			using rclass = RClass<>;
			using require_list = typename rclass::concept;
			constexpr static const bool rvalue = true;
			
			using type = U;
			using type_list = typename ExtractType<Type<U>>::type_list;
			using variant_type = typename VariantFuncList<type_list, type_list>::variant_type;

			variant_type var;
			template <typename T>
			FuncTypeWrapper& operator=(T func) {
				var = func;
			}
			template <typename F, typename ...TArgs>
			auto operator()(F&& f, TArgs&&... args) {
				if (var.empty())
					throw std::runtime_error("Not initialized.");
				auto func = var.template get<F>();
				return func(args...);
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

	// Finally wrapper all above into namespace and rename this

	using namespace def::lib;

	template <typename U>
	auto Ty = def::Type<U>{};

	template <typename U>
	auto Ls = def::List<U>{};
	
	template <typename ...Args>
	auto R = def::RClass<Args...>{};

	auto F = def::FuncTypeWrapperHead{};

	/* 
	Usage:
		For default function, int foo(char, int) ======> q = &foo; q(&foo, params...);
		For lambda function, [](char a) -> int   ======> q = &lambda::operator(); q(&lambda::operator(), params...);
	*/
}


#endif