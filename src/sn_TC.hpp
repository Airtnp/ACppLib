#ifndef SN_TC_H
#define SN_TC_H

#include "sn_CommonHeader.h"

// Type Calculus
namespace sn_TC {
	namespace basic_types {
		template <typename T>
		class One {

		};

		class Zero {

		};

		template <typename T>
		using Unit = One;

		using Void = Zero;

		template <typename T, typename U>
		class Union {
			friend class Union<U, T>;
		};

		template <typename T, typename U>
		using Multiply = Union<T, U>;

		template <typename T, typename U>
		class Either {
			friend class Either<U, T>;
		};

		

		template <typename T, typename U>
		using Add = Either<T, U>;

		template <typename T, typename U>
		inline auto operator+(T&& t, U&& u) {
			return Either<T, U>(std::forward<T>(t), std::forward<U>(u));
		}

		template <typename T, typename U>
		inline auto operator*(T&&, U&&) {
			return Union<T, U>{};
		}

	}
}



#endif