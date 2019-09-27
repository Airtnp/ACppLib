#ifndef SN_AOP_H
#define SN_AOP_H

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"
#include "sn_Macro.hpp"

namespace sn_AOP {


	namespace void_aspect {
		//ref: http://www.cnblogs.com/qicosmos/p/3154174.html

		class VoidAspect {
		public:
			template <typename F>
			VoidAspect(const F& f) : m_func(f) {	}

#if defined(SN_CONFIG_COMPILER_MSVC)
			SN_HAS_MEMBER_FUNCTION(before)
			SN_HAS_MEMBER_FUNCTION(after)
#else
            template <typename C, typename R, typename V = std::void_t<>, typename... Args>
            struct sn_has_member_function_before : std::false_type {};
            template <typename C, typename R, typename... Args>
            struct sn_has_member_function_before<C, R(Args...),
                    std::enable_if_t<
                            std::is_same<
                                    R, decltype(std::declval<C>().before(std::declval<Args>()...))
                            >::value, R
                    >, Args...> : std::true_type {};

            template <typename C, typename R, typename V = std::void_t<>, typename... Args>
            struct sn_has_member_function_after : std::false_type {};
            template <typename C, typename R, typename... Args>
            struct sn_has_member_function_after<C, R(Args...),
                    std::enable_if_t<
                            std::is_same<
                                    R, decltype(std::declval<C>().after(std::declval<Args>()...))
                            >::value, R
                    >, Args...> : std::true_type {};
#endif

				template <typename T>
			void invoke(T&& value) {
#if defined(_MSC_VER)
				__if_exists(value::before) {
					value.before();
				}
				m_func();
				__if_exists(value::after) {
					value.after();
				}
#else
				if (sn_has_member_function_before<T, void(void)>::value) {
					value.before();
				}
				m_func();
				if (sn_has_member_function_after<T, void(void)>::value) {
					value.after();
				}
#endif
			}

			template <typename H, typename... TArgs>
			void invoke(H&& head, TArgs&... tail) {
#if defined(_MSC_VER)
				__if_exists(head::before) {
					head.before();
				}
				invoke(std::forward<TArgs>(tail)...);
				__if_exists(head::after) {
					head.after();
				}
#elif defined(__GNUC__)
				if (sn_has_member_function_before<H, void(void)>::value) {
					head.before();
				}
				invoke(std::forward<TArgs>(tail)...);
				if (sn_has_member_function_after<H, void(void)>::value) {
					head.after();
				}
#endif
			}

		private:
			std::function<void()> m_func;
		};

		template <typename... APArgs>
		void invoke_void_aspect(const std::function<void()>& f) {
			VoidAspect map(f);
			map.invoke(APArgs()...);
		}
	}

	

}











#endif