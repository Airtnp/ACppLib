#ifndef SN_TYPELISP_TYPELIST_V1
#define SN_TYPELISP_TYPELIST_V1

#include <cstddef>

namespace sn_TypeLisp {

    template <typename T, typename U>
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

#ifndef __clang__
    template <template <typename ...TArgs> typename Op, typename ...FArgs>
    struct TypeCurry {
        template <typename ...LArgs>
        using type = Op<FArgs..., LArgs...>;
    };

    template <typename ...LArgs>
    template <template <typename ...TArgs> typename Op, typename ...FArgs>
    using TypeCurry_t = typename TypeCurry<Op, FArgs...>::template type<LArgs...>;
#endif

    /*
    Usage:
        template <typename ...LArgs>
        using AC = TypeCurry_t<Op, Args...>;
        
        using T = AC<Args...>;
    */

	template <int N>
	struct TypeNumber {
        static constexpr const int value = N; // sugar
    };

    // Lisp: quote/atom/eq/car/cdr/cons/cond

    struct TypeTrue {};

    // The only valid instance is TypeNil<>;
    template <typename ...Args>
    struct TypeNil {};

    // Actually this is useless, cpp will not evaluate (a b)
    // But lisp will evaluate, unless `(a, b), namely quote (a, b)
    template <typename T>
	struct TypeQuote {
		using type = T;
	};

	template <typename T>
	using TypeQuote_t = typename TypeQuote<T>::type;

	template <typename T>
	struct TypeAtom {
		using type = TypeTrue;
	};

	template <template <typename ...> typename TL, typename H, typename ...T>
	struct TypeAtom<TL<H, T...>> {
		using type = TL<>;
	};

	template <template <typename ...> typename TL>
	struct TypeAtom<TL<>> {
		using type = TypeTrue;
	};

	template <typename T>
	using TypeAtom_t = typename TypeAtom<T>::type;

	template <typename T, typename U>
	struct TypeEq {};

	template <typename T>
	struct TypeEq<T, T> {
		using type = TypeTrue;
	};

    template <template <typename ...> typename TL, typename ...Ts, typename ...Us>
    struct TypeEq<TL<Ts...>, TL<Us...>> {
        using type = TL<>;
    };

    template <template <typename ...> typename TL>
    struct TypeEq<TL<>, TL<>> {
        using type = TypeTrue;
    };

	template <template <typename ...> typename TL>
	struct TypeEq<TL<>, TypeNil<>> {
		using type = TypeTrue;
	};

	template <template <typename ...> typename TL>
	struct TypeEq<TypeNil<>, TL<>> {
		using type = TypeTrue;
	};

	template <typename T, typename U>
	using TypeEq_t = typename TypeEq<T, U>::type;

    template <typename T, typename U>
    struct TypeEqv {
        using type = TypeNil<>;
    };

    template <typename T>
	struct TypeEqv<T, T> {
		using type = TypeTrue;
	};

	template <template <typename ...> typename TL>
	struct TypeEqv<TL<>, TypeNil<>> {
		using type = TypeTrue;
	};

	template <template <typename ...> typename TL>
	struct TypeEqv<TypeNil<>, TL<>> {
		using type = TypeTrue;
	};

	template <typename T, typename U>
	using TypeEqv_t = typename TypeEqv<T, U>::type;

    template <typename T>
    struct TypeInv {};

    template <>
    struct TypeInv<TypeTrue> {
        using type = TypeNil<>;
    };

    template <template <typename ...> typename TL>
    struct TypeInv<TL<>> {
        using type = TypeTrue;
    };

    template <template <typename ...> typename TL, typename ...Ts>
    struct TypeInv<TL<Ts...>> {
        using type = TypeTrue;
    };

    template <typename T>
    using TypeInv_t = typename TypeInv<T>::type;

    template <typename T>
	struct TypeId {};

	template <template <typename ...> typename TL, typename ...Ts>
	struct TypeId<TL<Ts...>> {
		using type = TL<Ts...>;
	};

	template <typename T>
	using TypeId_t = typename TypeId<T>::type;

	template <typename B, typename T, typename F>
	struct TypeCond {};

	template <typename T, typename F>
	struct TypeCond<TypeTrue, T, F> {
		using type = T;
	};

	template <template <typename ...> typename TL, typename T, typename F>
	struct TypeCond<TL<>, T, F> {
		using type = F;
	};

	template <typename B, typename T, typename F>
	using TypeCond_t = typename TypeCond<B, T, F>::type;

    template <bool B, typename T, typename F>
	struct TypeCondv {};

	template <typename T, typename F>
	struct TypeCondv<true, T, F> {
		using type = T;
	};

	template <typename T, typename F>
	struct TypeCondv<false, T, F> {
		using type = F;
	};

	template <bool B, typename T, typename F>
	using TypeCondv_t = typename TypeCondv<B, T, F>::type;

    template <typename T>
	struct TypeCar;

	template <template <typename ...> typename TL, typename H, typename ...T>
	struct TypeCar<TL<H, T...>> {
		using type = H;
	};

	template <template <typename ...> typename TL>
	struct TypeCar<TL<>> {
		using type = TL<>;
	};

    template <typename T>
	using TypeCar_t = typename TypeCar<T>::type;

	template <typename T>
	struct TypeCdr;

	template <template <typename ...> typename TL, typename H, typename ...T>
	struct TypeCdr<TL<H, T...>> {
		using type = TL<T...>;
	};

	template <template <typename ...> typename TL, typename H>
	struct TypeCdr<TL<H>> {
		using type = TL<>;
	};

	template <template <typename ...> typename TL>
	struct TypeCdr<TL<>> {
		using type = TL<>;
	};

    template <typename T>
	using TypeCdr_t = typename TypeCdr<T>::type;

    template <typename T, typename U>
    struct TypeCons {};

    template <template <typename ...> typename TL, typename ...Ts, typename ...Us>
    struct TypeCons<TL<Ts...>, TL<Us...>> {
        using type = TL<Ts..., Us...>;
    };

    template <typename T, typename U>
    using TypeCons_t = typename TypeCons<T, U>::type;

	template <typename T>
	struct TypeLength;

	template <template <typename ...> typename TL>
	struct TypeLength<TL<>> {
		constexpr static const std::size_t value = 0;
	};

	template <template <typename ...> typename TL, typename H, typename ...T>
	struct TypeLength<TL<H, T...>> {
		// using type = TypeNumber<sizeof...(T) + 1>;
        constexpr static const std::size_t value = sizeof...(T) + 1; 
		// old-style
		// constexpr static const std::size_t value = 1 + TypeLength<TL<T...>>::value;
	};

    template <typename T>
	constexpr static const std::size_t TypeLength_v = TypeLength<T>::value;

	template <typename T, std::size_t N>
	struct TypeAt;

	template <template <typename ...> typename TL, std::size_t N>
	struct TypeAt<TL<>, N> {
		using type = TypeTrue;
	};

	template <template <typename ...> typename TL, typename H, typename ...T, std::size_t N>
	struct TypeAt<TL<H, T...>, N> {
		using type = TypeCondv_t<N == 0, H, typename TypeAt<TL<T...>, N - 1>::type>;
	};

    template <typename T, std::size_t N>
	using TypeAt_t = typename TypeAt<T, N>::type;

	template <typename T, typename ST>
	struct TypeIndex {
		constexpr static const int value = -1;
	};

	template <template <typename ...> typename TL, typename ...Args, typename ST>
	struct TypeIndex<TL<ST, Args...>, ST> {
		constexpr static const int value = 0;
	};

	template <template <typename ...> typename TL, typename H, typename ...T, typename ST>
	struct TypeIndex<TL<H, T...>, ST> {
		constexpr static const int value = (TypeIndex<TL<T...>, ST>::value == -1) ? -1 : 1 + (TypeIndex<TL<T...>, ST>::value);
	};

    template <typename L, typename ST>
	constexpr std::size_t TypeIndex_v = TypeIndex<L, ST>::value;

	template <typename L1, typename L2>
	struct TypeAppend;

/*
	template <template <typename ...> typename TL, typename ...T>
	struct TypeAppend<TL<T...>, TypeTrue> {
		using type = TL<T...>;
	};

    template <template <typename ...> typename TL, typename ...T>
	struct TypeAppend<TypeTrue, TL<T...>> {
		using type = TL<T...>;
	};
*/

	template <template <typename ...> typename TL, typename ...T, typename ST>
	struct TypeAppend<TL<T...>, ST> {
		using type = TL<T..., ST>;
	};

	template <template <typename ...> typename TL, typename ...T, typename ST>
	struct TypeAppend<ST, TL<T...>> {
		using type = TL<ST, T...>;
	};

	template <template <typename ...> typename TL, typename ...T1, typename ...T2>
	struct TypeAppend<TL<T1...>, TL<T2...>> {
		using type = TL<T1..., T2...>;
	};

    template <typename L1, typename L2>
	using TypeAppend_t = typename TypeAppend<L1, L2>::type;

	template <typename T, std::size_t N>
	struct TypeTake {};

	template <template <typename ...> typename TL, typename H, typename ...T, std::size_t N>
	struct TypeTake<TL<H, T...>, N> {
		using type = typename TypeAppend<H, typename TypeTake<TL<T...>, N-1>::type>::type;
	};

	template <template <typename ...> typename TL, typename ...T>
	struct TypeTake<TL<T...>, 0> {
		using type = TL<>;
	};

	template <typename T, std::size_t N>
	using TypeTake_t = typename TypeTake<T, N>::type;

	template <typename T, std::size_t N>
	struct TypeDrop {};

	template <template <typename ...> typename TL, typename H, typename ...T, std::size_t N>
	struct TypeDrop<TL<H, T...>, N> {
		using type = typename TypeDrop<TL<T...>, N-1>::type;
	};

	template <template <typename ...> typename TL, typename ...T>
	struct TypeDrop<TL<T...>, 0> {
		using type = TL<T...>;
	};

	template <typename T, std::size_t N>
	using TypeDrop_t = typename TypeDrop<T, N>::type;

	template <typename T, template <typename> class Op>
	struct TypeTakeWhile {};

	template <template <typename ...> typename TL, typename H, typename ...T, template <typename> class Op>
	struct TypeTakeWhile<TL<H, T...>, Op> {
		using type = TypeCond_t<typename Op<H>::type,
                                    typename TypeAppend<H, typename TypeTakeWhile<TL<T...>, Op>::type>::type,
                                    TL<H, T...>
                                >;
	};

	template <template <typename ...> typename TL, template <typename> typename Op>
	struct TypeTakeWhile<TL<>, Op> {
		using type = TL<>;
	};

	template <typename T, template <typename> class Op>
	using TypeTakeWhile_t = typename TypeTakeWhile<T, Op>::type;

	template <typename T, template <typename> class Op>
	struct TypeDropWhile {};

	template <template <typename ...> typename TL, typename H, typename ...T, template <typename> typename Op>
	struct TypeDropWhile<TL<H, T...>, Op> {
		using type = TypeCond_t<typename Op<H>::type,
                                    typename TypeDropWhile<TL<T...>, Op>::type,
                                    TL<H, T...>
                                >;
	};

	template <template <typename ...> typename TL, template <typename> typename Op>
	struct TypeDropWhile<TL<>, Op> {
		using type = TL<>;
	};

	template <typename T, template <typename> class Op>
	using TypeDropWhile_t = typename TypeDropWhile<T, Op>::type;

	template <typename T, template <typename> class Op>
	struct TypeMap {};

	template <template <typename ...> typename TL, typename H, typename ...T, template <typename> typename Op>
	struct TypeMap<TL<H, T...>, Op> {
		using type = typename TypeAppend<typename Op<H>::type, typename TypeMap<TL<T...>, Op>::type>::type;
	};

	template <template <typename ...> typename TL, template <typename> typename Op>
	struct TypeMap<TL<>, Op> {
		using type = TL<>;
	};

	template <typename T, template <typename> class Op>
	using TypeMap_t = typename TypeMap<T, Op>::type;

	template <typename T, template <typename> class Op>
	struct TypeFilter {};

	template <template <typename ...> typename TL, typename H, typename ...T, template <typename> typename Op>
	struct TypeFilter<TL<H, T...>, Op> {
		using type = TypeCond_t<typename Op<H>::type,
                                    typename TypeAppend<H, typename TypeFilter<TL<T...>, Op>::type>::type,
                                    typename TypeFilter<TL<T...>, Op>::type
                                >;
	};

	template <template <typename ...> typename TL, template <typename> typename Op>
	struct TypeFilter<TL<>, Op> {
		using type = TL<>;
	};

	template <typename T, template <typename> class Op>
	using TypeFilter_t = typename TypeFilter<T, Op>::type;

    // Foldr
    template <typename T, template <typename, typename> typename Op, typename Init>
    struct TypeFold {};

    template <template <typename ...> typename TL, typename H, typename ...T, template <typename, typename> typename Op, typename Init>
    struct TypeFold<TL<H, T...>, Op, Init> {
        using type = typename Op<H, typename TypeFold<TL<T...>, Op, Init>::type>::type;
    };

    template <template <typename ...> typename TL, typename T, template <typename, typename> typename Op, typename Init>
    struct TypeFold<TL<T>, Op, Init> {
        using type = typename Op<T, Init>::type;
    };

    template <typename T, template <typename, typename> typename Op, typename Init>
    using TypeFold_t = typename TypeFold<T, Op, Init>::type;

    template <typename L, typename T>
	struct TypeEraseAll;

	template <template <typename ...> typename TL, typename ...T, typename ST>
	struct TypeEraseAll<TL<T...>, ST> {
		template <typename SST>
		struct IsNotEqualST {
            // std::is_same
			// static constexpr const bool value = IsTypeSame<SST, ST>::value;
            using type = TypeInv_t<TypeEqv_t<SST, ST>>;
		};
		using type = typename TypeFilter<TL<T...>, IsNotEqualST>::type;
	};

    template <typename L, typename T>
	using TypeEraseAll_t = typename TypeEraseAll<L, T>::type;

    template <typename T>
	struct TypeUnique {};

	template <template <typename ...> typename TL>
	struct TypeUnique<TL<>> {
		using type = TL<>;
	};

	template <template <typename ...> typename TL, typename H, typename ...T>
	struct TypeUnique<TL<H, T...>> {
	private:
		using EraseAllHead = TypeEraseAll_t<TL<T...>, H>;
		using TailNoDuplicates = typename TypeUnique<EraseAllHead>::type;
	public:
		using type = TypeAppend_t<TL<H>, TailNoDuplicates>;
	};

    template <typename T>
	using TypeUnique_t = typename TypeUnique<T>::type;

	template <typename T, std::size_t N>
	struct TypeReplicate {};

	template <template <typename ...> typename TL, typename ...T, std::size_t N>
	struct TypeReplicate<TL<T...>, N> {
		using type = typename TypeAppend<TL<T...>, typename TypeReplicate<TL<T...>, N-1>::type>::type;
	};

	template <template <typename ...> typename TL, typename ...T>
	struct TypeReplicate<TL<T...>, 0> {
		using type = TL<>;
	};

    template <typename T, std::size_t N>
    using TypeReplicate_t = typename TypeReplicate<T, N>::type;

	template <typename T>
	struct Repeat {};

	template <template <typename ...> typename TL, typename H, typename ...T, std::size_t N>
	struct TypeTake<TL<Repeat<H>, T...>, N> {
		using type = typename TypeAppend<H, typename TypeTake<TL<Repeat<H>, T...>, N-1>::type>::type;
	};

	template <template <typename ...> typename TL, typename H, typename ...T, std::size_t N>
	struct TypeDrop<TL<Repeat<H>, T...>, N> {
		using type = typename TypeTake<TL<H, T...>, N-1>::type;
	};

	template <bool Statement, typename T1, typename T2>
	struct TypeIf {
		using type = T1;
	};

	template <typename T1, typename T2>
	struct TypeIf<false, T1, T2> {
		using type = T2;
	};

	template <bool Statement, typename T1, typename T2>
	using TypeIf_t = typename TypeIf<Statement, T1, T2>::type;

	template <typename ...TLs>
	struct TypeConcat {};

	template <typename H, typename ...Ls>
	struct TypeConcat<H, Ls...> {
		using type = TypeAppend_t<
							TypeId_t<H>,
							typename TypeConcat<Ls...>::type
						>;
	};

	template <typename L>
	struct TypeConcat<L> {
		using type = TypeId_t<L>;
	};

	template <typename ...Ts>
	using TypeConcat_t = typename TypeConcat<Ts...>::type;

	/*
		This is a good idea to simplify, but a bad idea regarding ADL

		template <template <typename ...> TL, typename ... Ts, typename ... Us>
        constexpr auto operator|(TL<Ts...>, TL<Us...>) {
            return TL<Ts..., Us...>{};
        }
	*/

	template <typename T, typename U>
	struct TypePair {};

	template <template <typename ...> typename TL, typename ...Ts, typename ...Us>
	struct TypePair<TL<Ts...>, TL<Us...>> {
		using type = TL<TL<Ts...>, TL<Us...>>;
		using left = TL<Ts...>;
		using right = TL<Us...>;
	};

	template <typename T, typename U>
	using TypePair_t = typename TypePair<T, U>::type;

	template <typename ...TLs>
	struct TypeTuple {};

	template <template <typename ...> typename TL, typename ...Ts, typename ...TLs>
	struct TypeTuple<TL<Ts...>, TLs...> {
		using type = TL<TL<Ts...>, TLs...>;
		template <std::size_t N>
		using type_n = TypeAt_t<type, N>;
	};

	template <typename ...TLs>
	using TypeTuple_t = typename TypeTuple<TLs...>::type;

	// or std::enable_if_t SFINAE
	template <typename T, std::size_t N, std::size_t M, bool V1 = (N == 0), bool V2 = (M == 0)>
	struct TypeSlice {};

	template <template <typename ...> typename TL, typename T, typename ...Ts, std::size_t N, std::size_t M>
	struct TypeSlice<TL<T, Ts...>, N, M, false, false> {
		static_assert(M <= sizeof...(Ts), "Index out of bound");
		static_assert(M >= N, "Back slicing is not implemented");
		using type = typename TypeSlice<TL<Ts...>, N-1, M-1>::type;
	};

	template <template <typename ...> typename TL, typename T, typename ...Ts, std::size_t M>
	struct TypeSlice<TL<T, Ts...>, 0, M, true, false> {
		using type = typename TypeAppend<TL<T>, typename TypeSlice<TL<Ts...>, 0, M-1>::type>::type;
	};

	template <template <typename ...> typename TL, typename ...Ts>
	struct TypeSlice<TL<Ts...>, 0, 0, true, true> {
		using type = TL<>;
	};

	template <typename T, std::size_t N, std::size_t M>
	using TypeSlice_t = typename TypeSlice<T, N, M>::type;

	template <typename T, std::size_t N, bool V = (N == 0)>
	struct TypeSplit {};

	template <template <typename ...> typename TL, typename T, typename ...Ts, std::size_t N>
	struct TypeSplit<TL<T, Ts...>, N, false> {
		static_assert(N <= sizeof...(Ts), "Index out of bound");
		using left = TypeAppend_t<TL<T>, typename TypeSplit<TL<Ts...>, N-1>::left>;
		using right = typename TypeSplit<TL<Ts...>, N-1>::right;
	};

	template <template <typename ...> typename TL, typename ...Ts>
	struct TypeSplit<TL<Ts...>, 0, true> {
		using left = TL<>;
		using right = TL<Ts...>;
	};

	// or use intermediate to reduce one template expansion
	template <typename T, std::size_t N>
	using TypeSplit_t = TypePair_t<typename TypeSplit<T, N>::left, typename TypeSplit<T, N>::right>;




}


#endif