#include "../sn_CommonHeader.h"

namespace sn_Assist {
	
    namespace sn_overload {
		
		// overload even can be used to detect member
		/* auto o = make_overload_func(
				[](auto&& c, auto&&... args) -> decltype(c.func_name(std::forward<Args>(args)...)) {
					return c.func_name(std::forward<Args>(args)...);
				},
				[](auto&& c, auto&&... args) {
					// do nothing
				}
			);
		*/

		// ref: https://www.zhihu.com/question/37202431
		// ref: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0051r2.pdf
		// In C++1z, you can straightly
		// class F : P...
		// using P::operator()... 
		// ref: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0195r2.html		
		template <typename F, typename...Args>
		struct func_overload_impl : F, func_overload_impl<Args...> {
			using F::operator();
			using func_overload_impl<Args...>::operator();
			func_overload_impl(F func_, Args... args_) : F(func_), func_overload_impl<Args...>(args_...) {}
		};

		template <typename F>
		struct func_overload_impl<F> : F {
			using F::operator();
			func_overload_impl(F func_) : F(func_) {}
		};

		template <typename ...Args>
		decltype(auto) make_overload_func(Args... args) {
			return func_overload_impl<Args...>{args...};
		}
	}
}