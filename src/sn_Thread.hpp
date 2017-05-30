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

#ifdef _WIN32
	// ref: https://gist.github.com/LYP951018/88eb616b939f19dad90bd6b087baa4d9
	namespace win32_handler {
		using Win32Handle = std::unique_ptr<void, void(*)(void *) noexcept>;
		Win32Handle make_win32handle(void* handle) {
			return Win32Handle(handle, [](void* h) noexcept {
				::CloseHandle(h);
			});
		}

		struct Done {};

		template <typename T>
		struct Payload {
			Win32Handle m_event;
			std::function<T()> m_func;
			std::variant<std::monostate, std::exception_ptr, T, Done> m_data;

			template <typename F>
			Payload(F f)
				: m_func{std::move(f)}, 
				m_event(make_win32handle(::CreateEventW({}, FALSE, FALSE, {}))) {
					assert(m_event);
			}
		};

		template <typename T>
		class Future {
		public:
			T get() {
				const auto wait_result = WaitForSingleObject(m_payload->m_event.get(), INFINITE);
				if (wait_result == WAIT_OBJECT_0) {
					switch(m_payload->m_data.index()) {
						case 1:
							done();
							std::rethrow_exception(std::get<1>(m_payload->m_data));
						case 2: {
							const auto result = std::get<2>(std::move(m_payload->m_data));
							done();
							return result;
						}
						default:
							std::terminate();
					}
				}
				else {
					throw std::system_error{static_cast<int>(::GetLastError()), std::system_category()};
				}
			}
			
			~Future() {
				if (m_payload->m_data.index() != 3) {
					WaitForSingleObject(m_payload->m_event.get(), INFINITE);
					done();
				}
			}

			Future(std::unique_ptr<Payload<T>> payload)
				: m_payload{std::move(payload)} {}

			Future(const Future& rhs)
				: m_payload{rhs.m_payload} {}

			Future(Future&& rhs)
				: m_payload{std::move(rhs.m_payload)} {}

		private:

			void done() {
				m_payload->m_data = Done{};
			}

			template <typename F>
			friend Future<T> async(F f);

			std::unique_ptr<Payload<T>> m_payload;
		};

		// std::invoke_result_of<F> = std::result_of_t<F()>
		// better use sn_Assist::sn_function_traits instead
		template <typename F>
		Future<std::result_of_t<F()>> async(F f) {
			using result_t = std::result_of_t<F()>;
			auto payload = std::make_unique<Payload<result_t>>(std::move(f));
			::QueueUserWorkItem([](void* param) -> DWORD {
				const auto p = static_cast<Payload<result_t>*>(param);
				{
					try {
						p->m_data = std::invoke(p->m_func);
					}
					catch (...) {
						p->m_data = std::current_exception();
					}
				}
				SetEvent(p->m_event.get());
				return 0;
			}, payload.get(), WT_EXECUTEDEFAULT);
			return Future<result_t>{std::move(payload)};
		}
	}

#endif


}



#endif