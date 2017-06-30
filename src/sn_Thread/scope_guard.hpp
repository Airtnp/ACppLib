#ifndef SN_THREAD_SCOPE_GUARD_H
#define SN_THREAD_SCOPE_GUARD_H

#include "../sn_CommonHeader.h"
#include "../sn_Assist.hpp"

namespace sn_Thread {
	namespace scope_guard {
		template <typename F, typename ...Args>
		class ScopeGuard final {
		public:
			constexpr ScopeGuard(F&& func, Args&&... args) noexcept : m_func{ std::move(func) }, m_args{ std::forward<Args>(args)... }, m_dismiss{ false } {}
			constexpr ScopeGuard(ScopeGuard&& rhs) noexcept : m_func{ std::move(rhs.m_func) }, m_args{ std::move(rhs.m_args) }, m_dismiss{ rhs.m_dismiss } {
				rhs.dismiss();
			}

			/*
			ScopeGuard& operator=(ScopeGuard&& rhs) noexcept {
				m_func = std::move(rhs.m_func);
				m_args = std::move(rhs.m_args);
				m_dismiss = rhs.m_dismiss;
				rhs.dismiss();
			}*/

			~ScopeGuard() {
				if (!m_dismiss)
					sn_Assist::sn_tuple_assist::invoke_tuple(m_func, m_args);
			}
			void dismiss() {
				m_dismiss = true;
			}
		private:
			F m_func;
			bool m_dismiss;
			std::tuple<const Args&...> m_args;
		};

		// Defect: Unfortunately, this is not guaranteed to be moving ctor. Use Args... will failed.
		// Update 1: Change to decltype(args)... can compile in gcc/clang (WTF), but still failed to compile in VS2015 (WTF*2)
		// Update 2: It seems class with explicit ctor in MSVC will have no moving semantics, and with template it will have tons of erros.
		template <typename F, typename ...Args>
		constexpr auto make_scope(F&& func, Args&&... args) {
			return ScopeGuard<F, decltype(args)...>{ std::move(func), std::forward<Args>(args)... };
		}

	}
}

#endif