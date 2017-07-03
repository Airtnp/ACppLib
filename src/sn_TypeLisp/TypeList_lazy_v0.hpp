#ifndef SN_TYPELISP_TYPELIST_V1
#define SN_TYPELISP_TYPELIST_V1

#include <cstddef>

// TODO: now the defintion seems ill-formed
// ref: https://stackoverflow.com/questions/36997351/using-a-template-before-its-specialized?rq=1
// define all in lazy mode
namespace sn_TypeLisp {
    template <typename ...Ts>
    using TypeVoid_t = void;

    template <typename T, typename U>
	struct TypeIsSame {
		constexpr static const bool value = false;
	};
	template <typename T>
	struct TypeIsSame<T, T> {
		constexpr static const bool value = true;
	};

    template <typename T, typename U>
    constexpr static const bool TypeIsSame_v = TypeIsSame<T, U>::value;

    struct TypeTrue {};

    // The only valid instance is TypeNil<>;
    template <typename ...Args>
    struct TypeNil {};


#ifdef __GNUC__

    /*
    Usage:
        template <typename ...LArgs>
        using AC = TypeCurry_t<Op, Args...>;
        
        using T = AC<Args...>;

    Or more general:
        template <typename ...LArgs>
        using T = typename TypeCurryProxy<LArgs...>::template type<Op, Args...>;
    */


    template <template <typename ...TArgs> typename Op, typename ...FArgs>
    struct TypeCurry {
        template <typename ...LArgs>
        using type = Op<FArgs..., LArgs...>;
    };

    template <typename ...LArgs>
    struct TypeCurryProxy {
        template <template <typename ...TArgs> typename Op, typename ...FArgs>
        using type = typename TypeCurry<Op, FArgs...>::template type<LArgs...>;    
    };

    /*
        template <typename ...LArgs>
        template <template <typename ...TArgs> typename Op, typename ...FArgs>
        using TypeCurry_t = typename TypeCurry<Op, FArgs...>::template     type<LArgs...>;
    */


    // wrapper
    template <template <typename ...> typename Op>
    struct TypeLazy {
        template <typename ...Ts>
        struct Lazy {
            using lazy = TypeTrue;
            template <typename ...Ts_>
            using type = Op<Ts_...>;
        };
        template <typename ...Ts>
        using type = Lazy<Ts...>;
    };

    template <typename ...Ts>
    struct TypeLazyProxy {
        template <template <typename ...> typename Op>
        using type = typename TypeCurry<Op>::template type<Ts...>;    
    };

    /*
        template <typename ...Ts>
        template <template <typename ...> typename Op>
        using TypeLazy_t = typename TypeLazy<Op>::template type<Ts...>;
    */

#endif


	template <int N>
	struct TypeNumber {
        static constexpr const int value = N;
    };

    template <typename T, typename U>
    struct TypeAdd {};

    template <typename T, typename U>
    struct TypeAddL {
        using lazy = TypeTrue;
        template <typename T_, typename U_>
        using type = TypeAdd<T_, U_>;  
    };

    template <int N1, int N2>
    struct TypeAdd<TypeNumber<N1>, TypeNumber<N2>> {
        using type = TypeNumber<N1 + N2>;
    };

    template <typename T, typename U>
    struct TypeSub {};

    template <typename T, typename U>
    struct TypeSubL {
        using lazy = TypeTrue;
        template <typename T_, typename U_>
        using type = TypeSub<T_, U_>;  
    };

    template <int N1, int N2>
    struct TypeSub<TypeNumber<N1>, TypeNumber<N2>> {
        using type = TypeNumber<N1 - N2>;
    };
    
    template <typename T, typename V = void, typename W = void>
    struct TypeForce {
        using type = T;
    };

    // ensure the typelist has type to itself?
    template <template <typename ...> typename Op, typename ...Ts, typename W>
    struct TypeForce<Op<Ts...>, TypeVoid_t<typename Op<Ts...>::lazy>, W> {
        using id = Op<Ts...>;
        using type = typename TypeForce<typename Op<Ts...>::template type<typename TypeForce<Ts>::type...>::type>::type;
    };

    template <template <typename ...> typename Op, typename ...Ts, typename V>
    struct TypeForce<Op<Ts...>, V, TypeVoid_t<typename Op<Ts...>::type>> {
        using type = typename TypeForce<typename Op<typename TypeForce<Ts>::type...>::type>::type;
    };


    template <template <typename ...> typename TL, typename ...Ts, typename V, typename W>
    struct TypeForce<TL<Ts...>, V, W> {
        using type = TL<typename TypeForce<Ts>::type...>;
    };

    template <typename T>
    using TypeEval = typename TypeForce<T>::type;

    // Lisp: quote/atom/eq/car/cdr/cons/cond

    // Actually this is useless, cpp will not evaluate (a b)
    // But lisp will evaluate, unless `(a, b), namely quote (a, b)
    template <typename T>
	struct TypeQuote {
		using type = T;
	};

	template <typename T>
	struct TypeAtom {
		using type = TypeTrue;
	};

    template <typename T>
    struct TypeAtomL {
        using lazy = TypeTrue;
        template <typename T_>
        using type = TypeAtom<T_>;  
    };

	template <template <typename ...> typename TL, typename H, typename ...T>
	struct TypeAtom<TL<H, T...>> {
		using type = TL<>;
	};

	template <template <typename ...> typename TL>
	struct TypeAtom<TL<>> {
		using type = TypeTrue;
	};

	template <typename T, typename U>
	struct TypeEq {
        using type = TypeNil<>;
    };

    template <typename T, typename U>
    struct TypeEqL {
        using lazy = TypeTrue;
        template <typename T_, typename U_>
        using type = TypeEq<T_, U_>;  
    };

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

    template <typename T>
    struct TypeInv {};

    template <typename T>
    struct TypeInvL {
        using lazy = TypeTrue;
        template <typename T_>
        using type = TypeInv<T_>;  
    };

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
	struct TypeId {};

    template <typename T>
    struct TypeIdL {
        using lazy = TypeTrue;
        template <typename T_>
        using type = TypeId<T_>;  
    };

	template <template <typename ...> typename TL, typename ...Ts>
	struct TypeId<TL<Ts...>> {
		using type = TL<Ts...>;
	};

	template <typename B, typename T, typename F>
	struct TypeCond {};

    template <typename B, typename T, typename F>
    struct TypeCondL {
        using lazy = TypeTrue;
        template <typename B_, typename T_, typename F_>
        using type = TypeCond<B_, T_, F_>;
    };

	template <typename T, typename F>
	struct TypeCond<TypeTrue, T, F> {
		using type = T;
	};

	template <template <typename ...> typename TL, typename T, typename F>
	struct TypeCond<TL<>, T, F> {
		using type = F;
	};

    template <typename T>
	struct TypeCar {};

    template <typename T>
    struct TypeCarL {
        using lazy = TypeTrue;
        template <typename T_>
        using type = TypeCar<T_>;  
    };

	template <template <typename ...> typename TL, typename H, typename ...T>
	struct TypeCar<TL<H, T...>> {
		using type = H;
	};

	template <template <typename ...> typename TL>
	struct TypeCar<TL<>> {
		using type = TL<>;
	};

	template <typename T>
	struct TypeCdr {};

    template <typename T>
    struct TypeCdrL {
        using lazy = TypeTrue;
        template <typename T_>
        using type = TypeCdr<T_>;  
    };

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

    template <typename T, typename U>
    struct TypeCons {};

    template <typename T, typename U>
    struct TypeConsL {
        using lazy = TypeTrue;
        template <typename T_, typename U_>
        using type = TypeCons<T_, U_>;  
    };

    template <template <typename ...> typename TL, typename ...Ts, typename ...Us>
    struct TypeCons<TL<Ts...>, TL<Us...>> {
        using type = TL<Ts..., Us...>;
    };

	template <typename T>
	struct TypeLength {};

    template <typename T>
    struct TypeLengthL {
        using lazy = TypeTrue;
        template <typename T_>
        using type = TypeLength<T_>;  
    };

	template <template <typename ...> typename TL>
	struct TypeLength<TL<>> {
		using type = TypeNumber<0>;
	};

	template <template <typename ...> typename TL, typename H, typename ...T>
	struct TypeLength<TL<H, T...>> {
		using type = TypeNumber<sizeof...(T) + 1>;
	};

    template <typename T, typename N>
	struct TypeAt {};

    template <typename T, typename U>
    struct TypeAtL {
        using lazy = TypeTrue;
        template <typename T_, typename U_>
        using type = TypeAt<T_, U_>;  
    };

	template <template <typename ...> typename TL, typename N>
	struct TypeAt<TL<>, N> {
		using type = TypeTrue;
	};

	template <template <typename ...> typename TL, typename H, typename ...T, typename N>
	struct TypeAt<TL<H, T...>, N> {
		using type = TypeCondL<
                            TypeEqL<N, TypeNumber<0>>, 
                            H, TypeAtL<
                                TL<T...>, 
                                TypeSubL<N, TypeNumber<1>>
                            >
                        >;
	};

	template <typename T, typename ST>
	struct TypeIndex {
        using type = TypeNumber<-1>;
	};

    template <typename T, typename ST>
    struct TypeIndexL {
        using lazy = TypeTrue;
        template <typename T_, typename ST_>
        using type = TypeIndex<T_, ST_>;    
    };

	template <template <typename ...> typename TL, typename ...Args, typename ST>
	struct TypeIndex<TL<ST, Args...>, ST> {
        using type = TypeNumber<0>;
	};

	template <template <typename ...> typename TL, typename H, typename ...T, typename ST>
	struct TypeIndex<TL<H, T...>, ST> {
		// constexpr static const int value = (TypeIndex<TL<T...>, ST>::value == -1) ? -1 : 1 + (TypeIndex<TL<T...>, ST>::value);
        using type = TypeCondL<
                            TypeEqL<
                                TypeIndexL<TL<T...>, ST>, TypeNumber<-1>
                            >,
                            TypeNumber<-1>,
                            TypeAddL<
                                TypeIndexL<TL<T...>, ST>, TypeNumber<1>
                            >
                        >;
	};

	template <typename L1, typename L2>
	struct TypeAppend {};

    template <typename L1, typename L2>
	struct TypeAppendL {
        using lazy = TypeTrue;
        template <typename L1_, typename L2_>
        using type = TypeAppend<L1_, L2_>;
    };

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

	
	template <typename T, typename N, bool V = (TypeIsSame_v<N, TypeNumber<0>>)>
	struct TypeTake {};

    // Or:
    // template <typename ...Ts>
    // using TypeAppendL = TypeLazy<TypeAppend>;
    // Or:
    // template <typename ...Ts>
    // using TypeAppendL = typename TypeLazyProxy<Ts...>::template type<TypeAppend>;
    template <typename T, typename N>
    struct TypeTakeL {
        using lazy = TypeTrue;
        template <typename T_, typename N_>
        using type = TypeTake<T_, N_>;  
    };


	template <template <typename ...> typename TL, typename H, typename ...T, typename N>
	struct TypeTake<TL<H, T...>, N, false> {
		using type = TypeAppendL<H, TypeTakeL<TL<T...>, TypeSubL<N, TypeNumber<1>>>>;
	};

	template <template <typename ...> typename TL, typename ...T>
	struct TypeTake<TL<T...>, TypeNumber<0>, true> {
		using type = TL<>;
	};

    template <template <typename ...> typename TL, typename N>
	struct TypeTake<TL<>, N, false> {
		using type = TL<>;
	};

	template <typename T, typename N, bool V = (TypeIsSame_v<N, TypeNumber<0>>)>
	struct TypeDrop {};

    template <typename T, typename N>
    struct TypeDropL {
        using lazy = TypeTrue;
        template <typename T_, typename N_>
        using type = TypeDrop<T_, N_>;  
    };

	template <template <typename ...> typename TL, typename H, typename ...T, typename N>
	struct TypeDrop<TL<H, T...>, N, false> {
		using type = TypeDropL<TL<T...>, TypeSubL<N, TypeNumber<1>>>;
	};

	template <template <typename ...> typename TL, typename ...T>
	struct TypeDrop<TL<T...>, TypeNumber<0>, true> {
		using type = TL<T...>;
	};

    template <template <typename ...> typename TL, typename N>
	struct TypeDrop<TL<>, N, false> {
		using type = TL<>;
	};

	template <typename T, template <typename> class Op>
	struct TypeTakeWhile {};

    template <typename T, template <typename> class Op>
    struct TypeTakeWhileL {
        using lazy = TypeTrue;
        template <typename T_, template <typename> class Op_>
        using type = TypeTakeWhile<T_, Op_>;  
    };

	template <template <typename ...> typename TL, typename H, typename ...T, template <typename> class Op>
	struct TypeTakeWhile<TL<H, T...>, Op> {
		using type = TypeCondL<Op<H>,
                            TypeAppendL<H, TypeTakeWhileL<TL<T...>, Op>>,
                            TL<H, T...>
                        >;
	};

	template <template <typename ...> typename TL, template <typename> typename Op>
	struct TypeTakeWhile<TL<>, Op> {
		using type = TL<>;
	};

	template <typename T, template <typename> class Op>
	struct TypeDropWhile {};

    template <typename T, template <typename> class Op>
    struct TypeDropWhileL {
        using lazy = TypeTrue;
        template <typename T_, template <typename> class Op_>
        using type = TypeDropWhile<T_, Op_>;  
    };

	template <template <typename ...> typename TL, typename H, typename ...T, template <typename> typename Op>
	struct TypeDropWhile<TL<H, T...>, Op> {
		using type = TypeCond<Op<H>,
                            TypeDropWhile<TL<T...>, Op>,
                            TL<H, T...>
                        >;
	};

	template <template <typename ...> typename TL, template <typename> typename Op>
	struct TypeDropWhile<TL<>, Op> {
		using type = TL<>;
	};

	template <typename T, template <typename> class Op>
	struct TypeMap {};

    template <typename T, template <typename> class Op>
    struct TypeMapL {
        using lazy = TypeTrue;
        template <typename T_, template <typename> class Op_>
        using type = TypeMap<T_, Op_>;  
    };

	template <template <typename ...> typename TL, typename H, typename ...T, template <typename> typename Op>
	struct TypeMap<TL<H, T...>, Op> {
		using type = TypeAppendL<Op<H>, TypeMapL<TL<T...>, Op>>;
	};

	template <template <typename ...> typename TL, template <typename> typename Op>
	struct TypeMap<TL<>, Op> {
		using type = TL<>;
	};

	template <typename T, template <typename> class Op>
	struct TypeFilter {};

    template <typename T, template <typename> class Op>
    struct TypeFilterL {
        using lazy = TypeTrue;
        template <typename T_, template <typename> class Op_>
        using type = TypeFilter<T_, Op_>;  
    };

	template <template <typename ...> typename TL, typename H, typename ...T, template <typename> typename Op>
	struct TypeFilter<TL<H, T...>, Op> {
		using type = TypeCondL<Op<H>,
                            TypeAppendL<H, TypeFilterL<TL<T...>, Op>>,
                            TypeFilterL<TL<T...>, Op>
                        >;
	};

	template <template <typename ...> typename TL, template <typename> typename Op>
	struct TypeFilter<TL<>, Op> {
		using type = TL<>;
	};

    // Foldr
    template <typename T, template <typename, typename> typename Op, typename Init>
    struct TypeFold {};

    template <typename T, template <typename, typename> typename Op, typename Init>
    struct TypeFoldL {
        using lazy = TypeTrue;
        template <typename T_, template <typename, typename> typename Op_, typename Init_>
        using type = TypeFold<T_, Op_, Init_>;
    };

    template <template <typename ...> typename TL, typename H, typename ...T, template <typename, typename> typename Op, typename Init>
    struct TypeFold<TL<H, T...>, Op, Init> {
        using type = Op<H, TypeFoldL<TL<T...>, Op, Init>>;
    };

    template <template <typename ...> typename TL, typename T, template <typename, typename> typename Op, typename Init>
    struct TypeFold<TL<T>, Op, Init> {
        using type = Op<T, Init>;
    };

    template <typename L, typename T>
	struct TypeEraseAll {};
    
    template <typename L, typename T>
	struct TypeEraseAllL {
        using lazy = TypeTrue;
        template <typename L_, typename T_>
	    using type = TypeEraseAll<L_, T_>;
    };
    

	template <template <typename ...> typename TL, typename ...T, typename ST>
	struct TypeEraseAll<TL<T...>, ST> {
		template <typename SST>
		struct IsNotEqualST {
            // std::is_same
            using type = TypeInvL<TypeEqL<SST, ST>>;
		};
        template <typename SST>
        struct IsNotEqualSTL {
            using lazy = TypeTrue;
            template <typename SST_>
            using type = IsNotEqualST<SST_>;
        };
		using type = TypeFilterL<TL<T...>, IsNotEqualST>;
	};

    template <typename T>
	struct TypeUnique {};

    template <typename T>
    struct TypeUniqueL {
        using lazy = TypeTrue;
        template <typename T_>
        using type = TypeUnique<T_>;
    };

	template <template <typename ...> typename TL>
	struct TypeUnique<TL<>> {
		using type = TL<>;
	};

	template <template <typename ...> typename TL, typename H, typename ...T>
	struct TypeUnique<TL<H, T...>> {
	private:
		using EraseAllHead = TypeEraseAllL<TL<T...>, H>;
		using TailNoDuplicates = TypeUniqueL<EraseAllHead>;
	public:
		using type = TypeAppendL<TL<H>, TailNoDuplicates>;
	};

	template <typename T, typename N, bool V = (TypeIsSame_v<N, TypeNumber<0>>)>
	struct TypeReplicate {};

    template <typename T, typename N>
    struct TypeReplicateL {
        using lazy = TypeTrue;
        template <typename T_, typename N_>
        using type = TypeReplicate<T_, N_>;
    };

	template <template <typename ...> typename TL, typename ...T, typename N>
	struct TypeReplicate<TL<T...>, N, false> {
		using type = TypeAppendL<TL<T...>, TypeReplicateL<TL<T...>, TypeSubL<N, TypeNumber<1>>>>;
	};

	template <template <typename ...> typename TL, typename ...T>
	struct TypeReplicate<TL<T...>, TypeNumber<0>, true> {
		using type = TL<>;
	};

	template <typename T>
	struct Repeat {};

	template <template <typename ...> typename TL, typename H, typename ...T, typename N>
	struct TypeTake<TL<Repeat<H>, T...>, N, false> {
		using type = TypeAppendL<H, TypeTakeL<TL<Repeat<H>, T...>, TypeSubL<N, TypeNumber<1>>>>;
	};

	template <template <typename ...> typename TL, typename H, typename ...T, typename N>
	struct TypeDrop<TL<Repeat<H>, T...>, N, false> {
		using type = TypeTakeL<TL<H, T...>, TypeSubL<N, TypeNumber<1>>>;
	};

	template <typename S, typename T1, typename T2>
	struct TypeIf {
		using type = T1;
	};

    template <typename S, typename T1, typename T2>
	struct TypeIfL {
        using lazy = TypeTrue;
        template <typename S_, typename T1_, typename T2_>
        using type = TypeIfL<S_, T1_, T2_>;
    };

	template <template <typename ...> typename TL, typename T1, typename T2>
	struct TypeIf<TL<>, T1, T2> {
		using type = T2;
	};

	template <typename ...TLs>
	struct TypeConcat {};

    template <typename ...TLs>
    struct TypeConcatL {
        using lazy = TypeTrue;
        template <typename ...TLs_>
        using type = TypeConcat<TLs_...>;
    };

	template <typename H, typename ...Ls>
	struct TypeConcat<H, Ls...> {
		using type = TypeAppendL<
							TypeIdL<H>,
							TypeConcatL<Ls...>
						>;
	};

	template <typename L>
	struct TypeConcat<L> {
		using type = TypeId<L>;
	};

	template <typename T, typename U>
	struct TypePair {};

    template <typename T, typename U>
    struct TypePairL {
        using lazy = TypeTrue;
        template <typename T_, typename U_>
        using type = TypePair<T_, U_>;
    };

	template <template <typename ...> typename TL, typename ...Ts, typename ...Us>
	struct TypePair<TL<Ts...>, TL<Us...>> {
		using type = TL<TL<Ts...>, TL<Us...>>;
		using left = TL<Ts...>;
		using right = TL<Us...>;
	};

	template <typename ...TLs>
	struct TypeTuple {};

    template <typename ...TLs>
    struct TypeTupleL {
        using lazy = TypeTrue;
        template <typename ...TLs_>
        using type = TypeTuple<TLs_...>;
    };

	template <template <typename ...> typename TL, typename ...Ts, typename ...TLs>
	struct TypeTuple<TL<Ts...>, TLs...> {
		using type = TL<TL<Ts...>, TLs...>;
		template <typename N>
		using get = TypeAtL<type, N>;
	};

}


#endif