#ifndef SN_TYPELIST_H
#define SN_TYPELIST_H

#include "sn_CommonHeader.h"

// ref: Modern Cpp Design/Loki
namespace sn_TypeLisp {

	template <typename T1, typename T2>
	struct IsTypeSame {
		constexpr static const bool value = false;
	};
	template <typename T>
	struct IsTypeSame<T, T> {
		constexpr static const bool value = true;
	};

	template <typename F, typename T>
	struct IsTypeConvertible {
	private:
		using Small = char;
		struct Big { char dummy[2]; };

		static Small test(T);   // or template <typename U> helper(T, U) helper(F, int)
		static Big test(...);
		static F makeF(); // or struct Conv { operator T(); operator U() const; };
	public:
		constexpr static const bool value = sizeof(test(makeF())) == sizeof(Small);
	};

	template <typename T>
	struct IsTypeConvertible<T, T> {
		constexpr static const bool value = true;
	};

	template <typename P, typename C>
	struct IsBaseClass {
		constexpr static const bool value = IsTypeConvertible<const C*, const P*>::value && !IsTypeSame<const P*, const void*>::value;
	};

	struct NullType {
		using type = NullType;
	};
	struct EmptyType;

	template <typename ...Ts>
	struct TypeList {};

	template <typename T>
	struct TypeCar;

	template <typename H, typename ...T>
	struct TypeCar<TypeList<H, T...>> {
		using type = H;
	};

	template <>
	struct TypeCar<TypeList<>> {
		using type = NullType;
	};

	template <typename T>
	using TypeCar_t = typename TypeCar<T>::type;

	template <typename T>
	struct TypeCdr;

	template <typename H, typename ...T>
	struct TypeCdr<TypeList<H, T...>> {
		using type = TypeList<T...>;
	};

	template <typename H>
	struct TypeCdr<TypeList<H>> {
		using type = NullType;
	};

	template <>
	struct TypeCdr<TypeList<>> {
		using type = NullType;
	};

	template <typename T>
	using TypeCdr_t = typename TypeCdr<T>::type;

	template <typename T>
	struct TypeLength;
	
	template <>
	struct TypeLength<TypeList<>> {
		constexpr static const std::size_t value = 0;
	};

	template <typename H, typename ...T>
	struct TypeLength<TypeList<H, T...>> {
		constexpr static const std::size_t value = 1 + TypeLength<TypeList<T...>>::value;
	};

	template <typename T>
	constexpr static const std::size_t TypeLength_v = TypeLength<T>::value;

	template <typename T, std::size_t N>
	struct TypeAt;

	template <std::size_t N>
	struct TypeAt<TypeList<>, N> {
		using type = NullType;
	};

	template <typename H, typename ...T, std::size_t N>
	struct TypeAt<TypeList<H, T...>, N> {
		using type = std::conditional_t<N == 0, H, typename TypeAt<TypeList<T...>, N - 1>::type>;
	};

	template <typename T, std::size_t N>
	using TypeAt_t = typename TypeAt<T, N>::type;

	template <typename T, typename ST>
	struct TypeIndex {
		constexpr static const int value = -1;
	};

	template <typename ...Args, typename ST>
	struct TypeIndex<TypeList<ST, Args...>, ST> {
		constexpr static const int value = 0;
	};

	template <typename H, typename ...T, typename ST>
	struct TypeIndex<TypeList<H, T...>, ST> {
		constexpr static const int value = (TypeIndex<TypeList<T...>, ST>::value == -1) ? -1 : 1 + (TypeIndex<TypeList<T...>, ST>::value);
	};

	template <typename L1, typename L2>
	struct TypeAppend;

	template <typename ...T>
	struct TypeAppend<TypeList<T...>, NullType> {
		using type = TypeList<T...>;
	};

	template <typename ...T, typename ST>
	struct TypeAppend<TypeList<T...>, ST> {
		using type = TypeList<T..., ST>;
	};

	template <typename ...T1, typename ...T2>
	struct TypeAppend<TypeList<T1...>, TypeList<T2...>> {
		using type = TypeList<T1..., T2...>;
	};

	template <typename L1, typename L2>
	using TypeAppend_t = typename TypeAppend<L1, L2>::type;

	template <typename L>
	struct TypeDropOne;

	template <typename T1, typename ...T>
	struct TypeDropOne<TypeList<T1, T...>> {
		using type = TypeAppend_t<TypeList<T1>, typename TypeDropOne<TypeList<T...>>::type>;
	};

	template <typename T1>
	struct TypeDropOne<TypeList<T1>> {
		using type = TypeList<>;
	};

	template <typename T>
	using TypeDropOne_t = typename TypeDropOne<T>::type;

	template <typename T>
	struct TypeReverse;

	template <typename H>
	struct TypeReverse<TypeList<H>> {
		using type = TypeList<H>;
	};

	template <>
	struct TypeReverse<TypeList<>> {
		using type = TypeList<>;
	};

	template <typename H, typename ...T>
	struct TypeReverse<TypeList<H, T...>> {
		using type = TypeAppend_t<typename TypeReverse<TypeList<T...>>::type, TypeList<H>>;
	};

	template <typename T>
	using TypeReverse_t = typename TypeReverse<T>::type;

	template <typename L, typename T>
	struct TypeErase;

	template <typename T>
	struct TypeErase<TypeList<>, T> {
		using type = TypeList<>;
	};

	template <typename ST, typename ...T>
	struct TypeErase<TypeList<ST, T...>, ST> {
		using type = TypeList<T...>;
	};

	template <typename H, typename ...T, typename ST>
	struct TypeErase<TypeList<H, T...>, ST> {
		using type = TypeAppend_t<TypeList<H>, typename TypeErase<TypeList<T...>, ST>::type>;
	};

	template <typename L, typename T>
	using TypeErase_t = typename TypeErase<L, T>::type;

	template <typename L, typename T>
	struct TypeEraseAll;

	template <typename T>
	struct TypeEraseAll<TypeList<>, T> {
		using type = TypeList<>;
	};

	template <typename ST, typename ...T>
	struct TypeEraseAll<TypeList<ST, T...>, ST> {
		using type = typename TypeEraseAll<TypeList<T...>, ST>::type;
	};

	template <typename H, typename ...T, typename ST>
	struct TypeEraseAll<TypeList<H, T...>, ST> {
		using type = TypeAppend_t<TypeList<H>, typename TypeEraseAll<TypeList<T...>, ST>::type>;
	};

	template <typename L, typename T>
	using TypeEraseAll_t = typename TypeEraseAll<L, T>::type;

	template <typename T>
	struct TypeEraseDuplicates;

	template <>
	struct TypeEraseDuplicates<TypeList<>> {
		using type = TypeList<>;
	};

	template <typename H, typename ...T>
	struct TypeEraseDuplicates<TypeList<H, T...>> {
	private:
		using EraseAllHead = TypeEraseAll_t<TypeList<T...>, H>;
		using TailNoDuplicates = typename TypeEraseDuplicates<EraseAllHead>::type;
	public:
		using type = TypeAppend_t<TypeList<H>, TailNoDuplicates>;
	};

	template <typename T>
	using TypeEraseDuplicates_t = typename TypeEraseDuplicates<T>::type;

	template <typename L, typename T, typename U>
	struct TypeReplace;

	template <typename T, typename U>
	struct TypeReplace<TypeList<>, T, U> {
		using type = TypeList<>;
	};

	template <typename ST, typename ...T, typename U>
	struct TypeReplace<TypeList<ST, T...>, ST, U> {
		using type = TypeList<U, T...>;
	};

	template <typename H, typename ...T, typename ST, typename U>
	struct TypeReplace<TypeList<H, T...>, ST, U> {
		using type = TypeAppend_t<TypeList<H>, typename TypeReplace<TypeList<T...>, ST, U>::type>;
	};

	template <typename L, typename T, typename U>
	using TypeReplace_t = typename TypeReplace<L, T, U>::type;

	template <typename L, typename T, typename U>
	struct TypeReplaceAll;

	template <typename T, typename U>
	struct TypeReplaceAll<TypeList<>, T, U> {
		using type = TypeList<>;
	};

	template <typename ST, typename ...T, typename U>
	struct TypeReplaceAll<TypeList<ST, T...>, ST, U> {
		using type = TypeAppend_t<TypeList<U>, typename TypeReplaceAll<TypeList<T...>, ST, U>::type>;
	};

	template <typename H, typename ...T, typename ST, typename U>
	struct TypeReplaceAll<TypeList<H, T...>, ST, U> {
		using type = TypeAppend_t<TypeList<H>, typename TypeReplaceAll<TypeList<T...>, ST, U>::type>;
	};

	template <typename L, typename T, typename U>
	using TypeReplaceAll_t = typename TypeReplaceAll<L, T, U>::type;

	template <typename L, typename T>
	struct TypeMostDerivedHelper;

	template <typename T>
	struct TypeMostDerivedHelper<TypeList<>, T> {
		using type = T;
	};

	template <typename H, typename ...T, typename ST>
	struct TypeMostDerivedHelper<TypeList<H, T...>, ST> {
	private:
		using Candidate = typename TypeMostDerivedHelper<TypeList<T...>, ST>::type;
	public:
		using type = std::conditional_t<std::is_base_of<Candidate, H>::value, H, Candidate>;
	};

	template <typename T>
	struct TypeMostDerived;

	template <>
	struct TypeMostDerived<TypeList<>> {
		using type = NullType;
	};

	template <typename H, typename ...T>
	struct TypeMostDerived<TypeList<H, T...>> {
		using type = typename TypeMostDerivedHelper<TypeList<H, T...>, H>::type;
	};

	template <typename T>
	using TypeMostDerived_t = typename TypeMostDerived<T>::type;

	template <typename T>
	struct TypeDerivedOrder;

	template <>
	struct TypeDerivedOrder<TypeList<>> {
		using type = NullType;
	};

	template <typename H, typename ...T>
	struct TypeDerivedOrder<TypeList<H, T...>> {
	private:
		using TheMostDerived = TypeMostDerived_t<TypeList<H, T...>>;
		using RemainType = TypeEraseAll<TypeList<H, T...>, TheMostDerived>;
	public:
		using type = TypeAppend<TypeList<TheMostDerived>, typename TypeDerivedOrder<RemainType>::type>;
	};

	template <typename T>
	using TypeDerivedOrder_t = typename TypeDerivedOrder<T>::type;

	template <typename T, std::size_t N>
	struct TypeRemoveN {};

	template <typename H, typename ...T, std::size_t N>
	struct TypeRemoveN<TypeList<H, T...>, N> {
		using type = typename TypeRemoveN<TypeList<T...>, N - 1>::type;
	};
	template <typename H, typename ...T>
	struct TypeRemoveN<TypeList<H, T...>, 0> {
		using type = TypeList<H, T...>;
	};
	template <typename L>
	struct TypeRemoveN<L, 0> {
		using type = L;
	};

	template <typename T, std::size_t N>
	using TypeRemoveN_t = typename TypeRemoveN<T, N>::type;


	template <typename T1, typename T2>
	using TypeCons = TypeAppend<T1, T2>;

	template <typename T1, typename T2>
	using TypeCons_t = TypeAppend_t<T1, T2>;

	template <typename T>
	struct Quote {
		using type = T;
	};

	template <typename T>
	using Quote_t = typename Quote<T>::type;

	template <typename T>
	struct Atom {
		using type = NullType;
	};

	template <typename H, typename ...T>
	struct Atom<TypeList<H, T...>> {
		using type = TypeList<>;
	};

	template <>
	struct Atom<TypeList<>> {
		using type = NullType;
	};

	template <typename T>
	using Atom_t = typename Atom<T>::type;

	template <typename T1, typename T2>
	struct Eq {
		using type = TypeList<>;
	};

	template <typename T>
	struct Eq<T, T> {
		using type = NullType;
	};

	template <typename T1, typename T2>
	using Eq_t = typename Eq<T1, T2>::type;

	template <typename L, typename V = void>
	struct Cond {};

	template <typename H, typename ...T>
	struct Cond<TypeList<H, T...>, std::void_t<std::enable_if_t<std::is_same<typename H::type, NullType>::value>>> {
		using type = H;
	};

	template <typename H, typename ...T>
	struct Cond<TypeList<H, T...>> {
		using type = typename Cond<TypeList<T...>>::type;
	};

	template <>
	struct Cond<TypeList<>> {
		using type = TypeList<>;
	};

	// Lisp: quote/atom/eq/car/cdr/cons/cond
}






#endif