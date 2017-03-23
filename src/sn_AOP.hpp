#ifndef SN_AOP_H
#define SN_AOP_H

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"
#include "sn_Macro.hpp"

namespace sn_AOP {
	//ref: http://www.cnblogs.com/qicosmos/p/3154174.html

	class sn_aspect {
	public:
		template <typename F>
		sn_aspect(const F& f) : m_func(f) {	}

#if defined(__GNUC__)
		sn_has_member_function(before)
		sn_has_member_function(after)
		//or if constexpr
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
#elif defined(__GNUC__)
			if (sn_has_member_function_value(value, before, void, void)) {
				value.before();
			}
			m_func();
			if (sn_has_member_function_value(value, after, void, void)) {
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
			if (sn_has_member_function_value(head, before, void, void)) {
				value.before();
			}
			m_func();
			if (sn_has_member_function_value(head, after, void, void)) {
				value.after();
			}
#endif
		}

	private:
		std::function<void()> m_func;
	};

	template <typename... APArgs>
	void invoke(const std::function<void()>& f) {
		sn_aspect map(f);
		map.invoke(APArgs()...);
	}

}











#endif