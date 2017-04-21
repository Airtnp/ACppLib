#ifndef SN_LC_ENCODING_H
#define SN_LC_ENCODING_H

#include "sn_CommonHeader.h"
#include "sn_LC.hpp"

// Not common encoding. Mogensen¨CScott/Church/Boehm-Berarducci
namespace sn_LCEncoding {
	using namespace sn_LC;
	namespace Church {
		using Nonsense = Literal<Zero>;
		enum { F, T };
		using ID = Lambda<T, Reference<T>>;
		template <typename FX, typename X>
		// lambda .f 
		//		lambda .x 
		//			x
		using ChurchZero = Application<Lambda<F, Application<ID, X>>, Nonsense>;
		// lambda .f 
		//		lambda .x 
		//			f 
		//			(lambda .f
		//				lambda .x
		//					f x ...)(f, x)
		template <std::size_t N, typename FX, typename X>
		struct ChurchNValue {
			using value = Application<
							Lambda<
								F,
								Application<
									Lambda<
										T,
										Application<
											Reference<F>,
											Reference<T>,
										>
									>,
									typename ChurchNValue<N - 1, FX, X>::value
								>
							>, FX>;
		};
		// lambda .f 
		//		lambda .x 
		//			f x
		template <typename FX, typename X>
		struct ChurchNValue<1, FX, X> {
			using value = Application<
							Lambda<
								F,
								Application<
									Lambda<
										T,
										Application<
											Reference<F>,
											Reference<T>,
										>
									>, 
									X
								>
							>, FX>;
		};
		template <typename FX, typename X>
		struct ChurchNValue<0, FX, X> {
			using value = Application<Lambda<F, Application<ID, X>>, Nonsense>;
		};
		template <std::size_t N, typename FX, typename X>
		using ChurchNumberT = typename ChurchNValue<N, FX, X>::value;
		
		// inc/pred/mult/sub is simple size_t operation
		template <std::size_t N, typename FX, typename X>
		struct ChurchSuccValue {
			using value = typename ChurchNValue<N + 1, FX, X>::value;
		};
		template <std::size_t N, typename FX, typename X>
		using ChurchSucc = typename ChurchSuccValue<N, FX, X>::value;

		template <typename FX, typename X>
		struct ChurchTValue {
			using value = FX;
		};
		template <typename FX, typename X>
		struct ChurchFValue {
			using value = X;
		};


		enum { V1, V2, V3, V4, V5,
				V6, V7, V8, V9, V10,
		};
		// lambda .f 
		//		lambda .x 
		//			f 
		//			(lambda .f
		//				lambda .x
		//					f x ...)(f, x)
		template <std::size_t N>
		struct ChurchNV {
			using value = VarLambda<
								VarList<V1, V2>,
								Application<
									Reference<V1>,
									Application<
										typename ChurchNV<N - 1>::value,
										ValList<Reference<V1>, Reference<V2>>
									>
								>
							>;
		};
		// lambda .f 
		//		lambda .x 
		//			f x
		template <>
		struct ChurchNV<1> {
			using value = VarLambda<
								VarList<V1, V2>,
								Application<
									Reference<V1>,
									Reference<V2>
								>
							>;
		};
		template <>
		struct ChurchNV<0> {
			using value = VarLambda<
								VarList<V1, V2>,
								Reference<V2>
							>;
		};

		template <std::size_t N, typename T>
		struct ChurchNImpl {
			constexpr static const std::size_t value = 
				std::is_same<typename ChurchNV<N>::value, T> ? N : ChurchNImpl<N + 1, T>::value;
		};


		template <std::size_t N>
		using ChurchNumber = typename ChurchNV<N>::value;
		// hate operations... so this is tricky and compiler-errno
		template <typename T>
		using ChurchN = ChurchNImpl<0, T>::value;
		
		using ChurchTrue = VarLambda<
								VarList<V1, V2>, 
								Reference<V1>
							>;
		using ChurchFalse = VarLambda<
								VarList<V1, V2>,
								Reference<V2>
							>;
		using ChurchAnd = VarLambda<
								VarList<V1, V2>,
								Application<
									Reference<V1>,
									ValList<Reference<V2>, Reference<V1>>
								>
							>;
		using ChurchOr = VarLambda<
								VarList<V1, V2>,
								Application<
									Reference<V1>,
									ValList<Reference<V1>, Reference<V2>>
								>
							>;
		using ChurchNot = VarLambda<
								VarList<V1>,
								Application<
									Reference<V1>,
									ValList<ChurchFalse, ChurchTrue>
								>
							>;
		using ChurchXor = VarLambda<
								VarList<V1, V2>, 
								Application<
									Reference<V1>,
									ValList<
										Application<
											ChurchNot,
											Reference<V2>
										>,
										Reference<V2>
									>
								>
							>;
		using ChurchIf = VarLambda<
							VarList<V1, V2, V3>,
							Application<
								Reference<V1>,
								ValList<
									Reference<V2>,
									Reference<V3>
								>
							>
						>;
		using ChurchIsZero = VarLambda<
								VarList<V1>,
								Application<
									Reference<V1>,
									ValList<
										Lambda<V2, ChurchFalse>,
										ChurchTrue
									>
								>
							>;
		using ChurchPair = VarLambda<
								VarList<V1, V2>,
								VarLambda<
									VarList<V3>,
									Application<
										Reference<V3>,
										ValList<
											Reference<V1>,
											Reference<V2>
										>
									>
								>
							>;
		using ChurchFirst = VarLambda<
								VarList<V1>,
								Application<
									Reference<V1>,
									ValList<ChurchTrue>
								>
							>;
		using ChurchSecond = VarLambda<
									VarList<V1>,
									Application<
										Reference<V1>,
										ChurchFalse
									>
							  >;
								
	}


	namespace Mogensen_Scott {

	}
}







#endif