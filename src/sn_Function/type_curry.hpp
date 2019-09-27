#ifndef SN_FUNCTION_TYPE_CURRY_H
#define SN_FUNCTION_TYPE_CURRY_H

namespace sn_Function {
    namespace template_currying {
		
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
			/*
				template <typename ...LArgs>
				struct LTemplate {
					using type = Op<Args..., LArgs...>;
				};
				template <typename ...LArgs>
				using type = typename LTemplate<LArgs...>::type;
			*/
			template <typename ...LArgs>
			using type = Op<FArgs..., LArgs...>;
		};

		template <typename ...LArgs>
		struct TypeCurryProxy {
			template <template <typename ...TArgs> typename Op, typename ...FArgs>
			using type = typename TypeCurry<Op, FArgs...>::template type<LArgs...>;    
		};

// No, standard forbids this
// ref: https://www.zhihu.com/question/61944238
#if defined(__GNUC__) && __GNUC__ < 8
			template <typename ...LArgs>
			template <template <typename ...TArgs> typename Op, typename ...FArgs>
			using TypeCurry_t = typename TypeCurry<Op, FArgs...>::template     type<LArgs...>;
#endif

        using TypeTrue = std::true_type;
        using TypeFalse = std::false_type;

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

#if defined(__GNUC__) && __GNUC__ < 8
			template <typename ...Ts>
			template <template <typename ...> typename Op>
			using TypeLazy_t = typename TypeLazy<Op>::template type<Ts...>;
#endif

	}
}


#endif