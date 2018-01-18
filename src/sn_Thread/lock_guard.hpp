#ifndef SN_THREAD_LOCK_GUARD_H
#define SN_THREAD_LOCK_GUARD_H

#include "../sn_CommonHeader.h"

namespace sn_Thread {
    
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

		template <typename T>
		class scope_thread {
			T& thd;
		public:
			scope_thread(T& t) : thd{t} {}
			~scope_thread {
				if (thd.joinable()) {
					thd.join();
				}
			}
		};

		// future destructor from packaged task doesn't block
		template< class Function, class... Args>
		std::future<typename std::result_of<Function(Args...)>::type> detach_async( Function&& f, Args&&... args ) {
			typedef typename std::result_of<Function(Args...)>::type R;
			auto bound_task = std::bind(std::forward<Function>(f), std::forward<Args>(args)...);
			std::packaged_task<R()> task(std::move(bound_task));
			auto ret = task.get_future();
			std::thread t(std::move(task));
			t.detach();
			return ret;   
		}

		// Another workaround

		template<class Function, class... Args>
		void async_wrapper(Function&& f, Args&&... args, std::future<void>& future,
						std::future<void>&& is_valid, std::promise<void>&& is_moved) {
			is_valid.wait(); // Wait until the return value of std::async is written to "future"
			auto our_future = std::move(future); // Move "future" to a local variable
			is_moved.set_value(); // Only now we can leave void_async in the main thread

			// This is also used by std::async so that member function pointers work transparently
			auto functor = std::bind(f, std::forward<Args>(args)...);
			functor();
		}

		template<class Function, class... Args> // This is what you call instead of std::async
		void void_async(Function&& f, Args&&... args) {
			std::future<void> future; // This is for std::async return value
			// This is for our synchronization of moving "future" between threads
			std::promise<void> valid;
			std::promise<void> is_moved;
			auto valid_future = valid.get_future();
			auto moved_future = is_moved.get_future();

			// Here we pass "future" as a reference, so that async_wrapper
			// can later work with std::async's return value
			future = std::async(
				async_wrapper<Function, Args...>,
				std::forward<Function>(f), std::forward<Args>(args)...,
				std::ref(future), std::move(valid_future), std::move(is_moved)
			);
			valid.set_value(); // Unblock async_wrapper waiting for "future" to become valid
			moved_future.wait(); // Wait for "future" to actually be moved
		}
	}
}

#endif