#include "../sn_CommonHeader.h"

namespace sn_Assist {
    // TODO: ref: YSLib/YBase/include/ystdex/cast.hpp
	namespace sn_cast {
		template <typename T, typename U, typename = U>
		struct static_dynamic_cast_impl{
			static U impl(T& t) {
				return dynamic_cast<U>(std::forward<T>(t));
			}
		};
		template <typename T, typename U>
		struct static_dynamic_cast_impl<T, U, decltype(static_cast<U>(std::declval<T>()))> {
			static U impl(T& t) {
				return static_cast<U>(std::forward<T>(t));
			}
		};

		template <typename T, typename U>
		constexpr U static_dynamic_cast(T&& t) {
			return static_dynamic_cast_impl<T, U>::impl(t);
		}

	}
}