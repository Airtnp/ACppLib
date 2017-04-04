#ifndef SN_THREAD_H
#define SN_THREAD_H

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"
//TODO: add wrapper of win/pthread
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

	namespace lock_guard {

		SN_HAS_MEMBER_FUNCTION(lock)
		SN_HAS_MEMBER_FUNCTION(unlock)

		
		template <typename T>
		struct IsLockableAndUnLockable {
			static const bool is_lockable = SN_HAS_MEMBER_FUNCTION_VALUE(T, lock, void, void);
			static const bool is_unlockable = SN_HAS_MEMBER_FUNCTION_VALUE(T, unlock, void, void);
			static const bool value = is_lockable && is_unlockable;
		};

		template <typename ...Args>
		class MultipleLockGuard : sn_Assist::sn_functional_base::nonmoveable {
		private:
			static_assert(std::conjunction<IsLockableAndUnLockable<Args>...>::value, "Locks should be lockable and unlockable");
			
			template <std::size_t a, std::size_t b, std::size_t step>
			struct GetNextConstant : std::conditional_t < a < b, std::integral_constant<std::size_t, a + step>, std::integral_constant<std::size_t, a - step>> {};

			template <std::size_t current, std::size_t target>
			struct Impl {
				template <typename T>
				static void lock(T&& tp) {
					std::get<current>(tp).lock();
					LockImpl<GetNextConstant<current, target>::value, target>::lock(tp);
				}

				template <typename T>
				static void unlock(T&& tp) {
					std::get<current>(tp).unlock();
					LockImpl<GetNextConstant<current, target>::value, target>::unlock(tp);
				}
			};

			template <std::size_t current>
			struct Impl<current, current> {
				template <typename T>
				static void lock(T&& tp) {
					std::get<current>(tp).lock();
				}

				template <typename T>
				static void unlock(T&& tp) {
					std::get<current>(tp).unlock();
				}
			};

		public:
			constexpr explicit MultipleLockGuard(Args&... locks) noexcept : m_locks{ locks... } {
				Impl<0, std::tuple_size<decltype(m_locks)>::value - 1>::lock(m_locks);
			}

			~MultipleLockGuard() noexcept {
				Impl<std::tuple_size<decltype(m_locks)>::value - 1, 0>::unlock(m_locks);
			}

		private:
			std::tuple<Args&...> m_locks;
		};
	}

}





#endif