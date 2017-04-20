#ifndef SN_REFLECTION_H
#define SN_REFLECTION_H

#include "sn_CommonHeader.h"
#include "sn_Macro.hpp"

namespace sn_Reflection {
	//ref: http://purecpp.org/?p=1074
	//VS2015 (no update?) gave little support on constexpr function (defined in C++1y)
#if defined(__GNUC__) || defined(__clang__)
	namespace unamed_pod_reflect {

		//-----------pod<->array-----------
		template <typename T>
		struct id {};

		/*
		template <typename T>
		struct type {
			using type = T;
		};
		*/

#define SN_REFLECTION_REGISTER_TYPE(Type, Index) \
		constexpr std::size_t type_to_id(id<Type>) noexcept { \
			return Index; \
		} \
		constexpr auto id_to_type(std::integral_constant<std::size_t, Index>) noexcept { \
			Type res{}; \
			return res; \
		} \


		SN_REFLECTION_REGISTER_TYPE(unsigned char     , 1)
		SN_REFLECTION_REGISTER_TYPE(unsigned short    , 2)
		SN_REFLECTION_REGISTER_TYPE(unsigned int      , 3)
		SN_REFLECTION_REGISTER_TYPE(unsigned long     , 4)
		SN_REFLECTION_REGISTER_TYPE(unsigned long long, 5)
		SN_REFLECTION_REGISTER_TYPE(signed char       , 6)
		SN_REFLECTION_REGISTER_TYPE(short             , 7)
		SN_REFLECTION_REGISTER_TYPE(int               , 8)
		SN_REFLECTION_REGISTER_TYPE(long              , 9)
		SN_REFLECTION_REGISTER_TYPE(long long         , 10)
		SN_REFLECTION_REGISTER_TYPE(char              , 11)
		SN_REFLECTION_REGISTER_TYPE(wchar_t           , 12)
		SN_REFLECTION_REGISTER_TYPE(double, 13)

		// Type{} cannot use void* (may use type<Type>{}::type instead)
		//SN_REFLECTION_REGISTER_TYPE(void*             , 13)
		//SN_REFLECTION_REGISTER_TYPE(const void*       , 14)
		
		
		template <typename T, std::size_t N>
		struct array {
			using type = T;
			T data[N];  //save id and its position

			static constexpr std::size_t size() noexcept {
				return N;
			}
		};

		template<std::size_t I>
		struct ubiq {
			std::size_t* ref_;

			template <typename T>
			constexpr operator T() const noexcept {
				ref_[I] = type_to_id(id<T>{});
				return T{};
			}
		};

		template <typename T, std::size_t... I>
		constexpr auto type_to_array_of_type_ids(std::size_t* types) noexcept
			-> decltype(T{ ubiq<I>{types}... }) {
			return T{ ubiq<T>{types}... };
		}

		template <typename Arr>
		constexpr auto count_nonzeros(Arr a) noexcept {
			std::size_t count = 0;
			for (std::size_t i = 0; i < Arr::size() && a.data[i]; ++i)
				++count;
			return count;
		}

		//expand index_sequence -> size_t parameter pack -> if not valid size_t, SFINAE
		template <typename T, std::size_t I0, std::size_t ...I>
		constexpr auto detect_fields_count_and_type_ids(std::size_t* types, std::index_sequence<I0, I...>) noexcept
			-> decltype(type_to_array_of_type_ids<T, I0, I...>(types))
		{
			return type_to_array_of_type_ids<T, I0, I...>(types);
		}

		template <typename T, std::size_t ...I>
		constexpr T detect_fields_count_and_type_ids(std::size_t* types, std::index_sequence<I...>) noexcept {
			return detect_fields_count_and_type_ids<T>(types, std::make_index_sequence<sizeof...(I)-1>{});
		}

		template <typename T>
		constexpr T detect_fields_count_and_type_ids(std::size_t*, std::index_sequence<>) noexcept {
			static_assert(sizeof(T), "Failed for unknown reason");
			return T{};
		}

		template <typename T>
		constexpr auto fields_count_and_type_ids_with_zeros() noexcept {
			static_assert(std::is_trivial<T>::value, "Not pod type");
			array<std::size_t, sizeof(T)> types{};
			detect_fields_count_and_type_ids<T>(types.data, std::make_index_sequence<sizeof(T)>{});
			return types;
		}

		template <typename T>
		constexpr auto array_of_type_ids() noexcept {
			constexpr auto types = fields_count_and_type_ids_with_zeros<T>();
			constexpr std::size_t count = count_nonzeros(types);
			array<std::size_t, count> res{};
			for (std::size_t i = 0; i < count; ++i) {
				res.data[i] = types.data[i];
			}
			return res;
		}

		//-----------pod<->tuple-----------

		template <std::size_t I, typename T, std::size_t N>
		constexpr const T& get(const array<T, N>& a) noexcept {
			return a.data[I];
		}

		template <typename T, std::size_t ...I>
		constexpr auto array_of_type_ids_to_index_sequence(std::index_sequence<I...>) noexcept {
			constexpr auto arr = array_of_type_ids<T>();
			return std::index_sequence<get<I>(a)...>{};
		}

		

		namespace pod_tuple {
			template <std::size_t N, typename T>
			struct base_from_member {
				T value;
			};

			template <typename I, typename ...Tail>
			struct tuple_base;

			template <std::size_t ...I, typename ...Tail>
			struct tuple_base<std::index_sequence<I...>, Tail...> : base_from_member<I, Tail>... {
				static constexpr std::size_t size_v = sizeof...(I);

				constexpr tuple_base() noexcept = default;
				constexpr tuple_base(tuple_base&&) noexcept = default;
				constexpr tuple_base(const tuple_base&) noexcept = default;

				constexpr tuple_base(Tail... v) noexcept : base_from_member<I, Tail>{ v }... {}
			};

			template <>
			struct tuple_base<std::index_sequence<>> {
				static constexpr std::size_t size_v = 0;
			};

			template <typename ...Args>
			struct tuple : tuple_base<std::make_index_sequence<sizeof...(Args)>, Args...> {
				using tuple_base<std::make_index_sequence<sizeof...(Args)>, Args...>::tuple_base;
			};

			template <std::size_t N, typename T>
			constexpr const T& get_impl(const base_from_member<N, T>& t) noexcept {
				return t.value;
			}

			template <std::size_t N, typename T>
			constexpr decltype(auto) get(const tuple<T...>& t) noexcept {
				static_assert(N < tuple<T...>::size_v, "Tuple index out of bounds");
				return get_impl<N>(t);
			}

		}

		template <std::size_t ...I>
		constexpr auto index_seq_as_tuple_impl(std::index_sequence<I...>) noexcept {
			return pod_tuple::tuple<decltype(id_to_type(std::integral_constant<std::size_t, I>{}))... > {};
			//return std::tuple<decltype(id_to_type(std::integral_constant<std::size_t, I>{}))... > {};
		}

		template <typename T>
		constexpr auto index_seq_as_tuple() noexcept {
			static_assert(std::is_pod<T>::value, "Not pod type");
			constexpr auto res = index_seq_as_tuple_impl(
				array_of_type_ids_to_index_sequence<T>(
					std::make_index_sequence<decltype(array_of_type_ids<T>())::size()>()
					)
			);
			static_assert(sizeof(res) == sizeof(T), "size check failed");
			static_assert(std::alignment_of<decltype(res)>::value == std::alignment_of<T>::value, "alignment check failed");
			return res;
		}

		template <std::size_t I, typename T>
		decltype(auto) get(const T& val) noexcept {
			auto t = reinterpret_cast<const decltype(index_seq_as_tuple<T>())*>(std::addressof(val));
			return get<I>(*t);
		}

	}
#endif

	//ref: https://github.com/qicosmos/iguana/blob/master/reflection.hpp
	namespace named_pod_reflect {
#define SN_REGISTER_ARG_LIST_1(op, arg, ...) op(arg)
#define SN_REGISTER_ARG_LIST_2(op, arg, ...) op(arg), MACRO_EXPAND(SN_REGISTER_ARG_LIST_1(op, __VA_ARGS__))
#define SN_REGISTER_ARG_LIST_3(op, arg, ...) op(arg), MACRO_EXPAND(SN_REGISTER_ARG_LIST_2(op, __VA_ARGS__))
#define SN_REGISTER_ARG_LIST_4(op, arg, ...) op(arg), MACRO_EXPAND(SN_REGISTER_ARG_LIST_3(op, __VA_ARGS__))
#define SN_REGISTER_ARG_LIST_5(op, arg, ...) op(arg), MACRO_EXPAND(SN_REGISTER_ARG_LIST_4(op, __VA_ARGS__))
#define SN_REGISTER_ARG_LIST_6(op, arg, ...) op(arg), MACRO_EXPAND(SN_REGISTER_ARG_LIST_5(op, __VA_ARGS__))
#define SN_REGISTER_ARG_LIST_7(op, arg, ...) op(arg), MACRO_EXPAND(SN_REGISTER_ARG_LIST_6(op, __VA_ARGS__))

#define SN_REGISTER_ID(ID) ID

#define SN_REGISTER_ARG_LIST(N, op, arg, ...) \
	MACRO_CONCAT(SN_REGISTER_ARG_LIST, N)(op, arg, __VA_ARGS__)

#define SN_SEPERATOR ,
#define SN_CONCAT_INIT_LIST_1(element, ...) #element
#define SN_CONCAT_INIT_LIST_2(element, ...) #element SN_SEPERATOR MACRO_EXPAND(SN_CONCAT_INIT_LIST_1(__VA_ARGS__))
#define SN_CONCAT_INIT_LIST_3(element, ...) #element SN_SEPERATOR MACRO_EXPAND(SN_CONCAT_INIT_LIST_2(__VA_ARGS__))
#define SN_CONCAT_INIT_LIST_4(element, ...) #element SN_SEPERATOR MACRO_EXPAND(SN_CONCAT_INIT_LIST_3(__VA_ARGS__))
#define SN_CONCAT_INIT_LIST_5(element, ...) #element SN_SEPERATOR MACRO_EXPAND(SN_CONCAT_INIT_LIST_4(__VA_ARGS__))
#define SN_CONCAT_INIT_LIST_6(element, ...) #element SN_SEPERATOR MACRO_EXPAND(SN_CONCAT_INIT_LIST_5(__VA_ARGS__))
#define SN_CONCAT_INIT_LIST_7(element, ...) #element SN_SEPERATOR MACRO_EXPAND(SN_CONCAT_INIT_LIST_6(__VA_ARGS__))

		template <typename T>
		struct sn_reflection_member {};

#define SN_GENERATE_META_DATA_IMPL(STRUCT_NAME, ...) \
	template <> \
	struct sn_reflection_member<STRUCT_NAME> { \
		constexpr decltype(auto) static apply() { \
			return std::make_tuple(__VA_ARGS__); \
		} \
		using type = void; \
		constexpr static const char* name = #STRUCT_NAME; \
		constexpr static const size_t value = SN_GET_ARG_N(__VA_ARGS__); \
		constexpr static const std::array<const char*, value>& arr = arr_##STRUCT_NAME; \
	};

#define SN_GENERATE_META_DATA(STRUCT_NAME, N, ...) \
	constexpr std::array<const char*, N> arr_##STRUCT_NAME  = {MACRO_EXPAND(MACRO_CONCAT(SN_CONCAT_INIT_LIST, N)(__VA_ARGS__))}; \
	SN_GENERATE_META_DATA_IMPL(STRUCT_NAME, SN_REGISTER_ARG_LIST(N, &STRUCT_NAME::SN_REGISTER_ID, __VA_ARGS__))

#define SN_REFLECTION(STRUCT_NAME, ...) \
	SN_GENERATE_META_DATA(STRUCT_NAME, SN_GET_ARG_N(__VA_ARGS__), __VA_ARGS__)

		/* 
		In actual application, just expose whole into global namespace
		Also, use tuple::apply and T.*get<N>(sn_reflection_member<T>::apply()) to access member

		template <typename T, typename = void>
		struct sn_is_reflection : std::false_type {};

		template <typename T>
		struct sn_is_reflection<T, std::void_t<sn_reflection_member<std::decay_t<T>>::type>> : std::true_type {};

		template <typename T, std::size_t I>
		constexpr const char* sn_reflection_get_name() {
			using M = sn_reflection_member<std::decay_t<T>>;
			static_assert(I<M::value>, "out of range");
			return M::arr[I];
		}

		template <typename T>
		constexpr const char* sn_reflection_get_name() {
			using M = sn_reflection_member<std::decay_t<T>>;
			return M::name;
		}

		template <typename T>
		constexpr const char* sn_reflection_get_name(std::size_t i) {
			using M = sn_reflection_member<std::decay_t<T>>;
			return i >= M::value ? "" : M::arr[i];
		}

		template <typename T>
		std::enable_if_t<sn_is_reflection<T>::value, std::size_t> sn_reflection_get_value() {
			using M = sn_reflection_member<std::decay_t<T>>;
			return M::value;
		}

		template <typename T>
		std::enable_if_t<!sn_is_reflection<T>::value, std::size_t> sn_reflection_get_value() {
			return 0;
		}
		


		*/
		
	}

}





#endif