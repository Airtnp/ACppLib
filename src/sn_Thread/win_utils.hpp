#ifndef SN_THREAD_WIN_UTILS_H
#define SN_THREAD_WIN_UTILS_H

#include "../sn_CommonHeader.h"

namespace sn_Thread {
    
#ifdef _WIN32 && SN_ENABLE_WINDOWS_API

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

	// ref: https://gist.github.com/LYP951018/36146c4eb3b0cef8cc526ef9e9543e3d
	// synchapi.h
	namespace fast_mutex {
		class mutex {
		public:
			void lock() {
				while (true) {
					std::uint32_t waiters = m_waiters.load(std::memory_order::memory_order_relaxed);
					if ((waiters & 1) == 0)
						break;
					if (m_waiters.compare_exchange_strong(waiters, waiters + kWait)) {
						WaitOnAddress(&m_waiters, &waiters, 4, INFINITE);
					}
				}
			}

			void unlock() noexcept {
				std::uint32_t waiters = m_waiters.load(std::memory_order::memory_order_relaxed);
				while (!m_waiters.compare_exchange_strong(waiters, waiters & ~1u))
					;
				while (true) {
					waiters = m_waiters;
					if (waiters < kWait)
						return;
					if (waiters & 1)
						return;
					if (m_waiters.compare_exchange_strong(waiters, waiters - kWait)) {
						WakeByAddressSingle(const_cast<std::atomic<std::uint32_t>*>(&m_waiters));
						break;
					}
				}
			}
		private:
			enum : std::uint32_t {
				kWait = 512;
			};
			volatile std::atomic<std::uint32_t> m_waiters{ 0 };
		};
	}

#endif


}



#endif