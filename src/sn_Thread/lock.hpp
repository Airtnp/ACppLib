#ifndef SN_THREAD_LOCK_H
#define SN_THREAD_LOCK_H

#include <mutex>
#include <thread>
#include <exception>
#include <climits>

namespace sn_Thread {
    namespace lock {
        class hierarchy_mutex {
            std::mutex m_mtx;
            const uint64_t m_hv;
            const uint64_t m_prev_hv;
            static thread_local uint64_t this_thread_hv;

            void check_hv_violation() {
                if (this_thread_hv <= m_hv) {
                    throw std::logic_error("Mutex hierachy violated")
                }
            }

            void update_hv() {
                m_prev_hv = this_thread_hv;
                this_thread_hv = m_hv;
            }

        public:
            explicit hierarchy_mutex(uint64_t v) : m_hv{v}, m_prev_hv{0} {}

            void lock() {
                check_hv_violation();
                m_mtx.lock();
                update_hv();
            }

            void unlock() {
                this_thread_hv = m_prev_hv;
                m_mtx.unlock();
            }

            bool try_lock() {
                check_hv_violation();
                if (!m_mtx.try_lock())
                    return false;
                update_hv();
                return true;
            }
        };
        thread_local uint64_t hierarchy_mutex::this_thread_hv(ULONG_MAX);

        class spinlock_mutex {
			std::atomic_flag flg = ATOMIC_FLAG_INIT;
		public:
			template <typename F = void()>
			void lock(F idle == []{}) {
				while (flg.test_and_set(std::memory_order_acquire)) {
					idle();
				}
			}
			
			void unlock() {
				flg.clear(std::memory_order_release);
			}

			spinlock_mutex() = default;
			spinlock_mutex(const spinlock_mutex &) = delete;
			spinlock_mutex & operator = (const spinlock_mutex &) = delete;
		};

        #define cpu_relax() \
            __asm__ __volatile__ ( "pause\n" : : : "memory")

        inline int32_t cmpxchg(int32_t* ptr, int32_t i_old, int32_t i_new) {
            int32_t ret;
            __asm__ __volatile__ (
                "lock\n" "cmpxchgl %2,%1\n"
                : "=a" (ret), "+m" (*ptr)
                : "r" (i_new), "0" (i_old)
                : "memory");
            return ret;
        }

        inline int32_t xchg(int32_t* ptr, int32_t x) {
            __asm__ __volatile__ (
                "lock\n" "xchgl %0,%1\n"
                :"=r" (x), "+m" (*ptr)
                :"0" (x)
                :"memory");
            return x;
        }

        inline int sys_futex( void * addr, std::int32_t op, std::int32_t x) {
            return ::syscall( SYS_futex, addr, op, x, nullptr, nullptr, 0);
        }

        inline int futex_wake( std::atomic< std::int32_t > * addr) {
            return 0 <= sys_futex( static_cast< void * >( addr), FUTEX_WAKE_PRIVATE, 1) ? 0 : -1;
        }

        inline int futex_wait( std::atomic< std::int32_t > * addr, std::int32_t x) {
            return 0 <= sys_futex( static_cast< void * >( addr), FUTEX_WAIT_PRIVATE, x) ? 0 : -1;
        }

        class futex_mutex {
            int32_t mtx;
            static const constexpr size_t spin_n = 100;
            enum class mtx_state {
                MTX_FREE = 0,
                MTX_LOCKED = 1,
                MTX_LOCKED_AND_CONTESTED = 2
            };
        public:
            void lock() {
                int32_t c;
                for (size_t i = 0; i < spin_n; ++i) {
                    c = cmpxchg(&mtx, MTX_FREE, MTX_LOCKED);
                    if(c == MTX_FREE) {
                        return;
                    }
                    cpu_relax();
                }
                if (c == MTX_LOCKED) {
                    c = xchg(&mtx, MTX_LOCKED_AND_CONTESTED);
                }
                while (c) {
                    syscall(SYS_futex, &mtx, FUTEX_WAIT, MTX_LOCKED_AND_CONTESTED, NULL, NULL, 0);
                    c = xchg(&mtx, MTX_LOCKED_AND_CONTESTED);
                }
            }

            void unlock() {
                if (mtx == MTX_LOCKED_AND_CONTESTED) {
                    mtx == MTX_FREE;
                } else if (xchg(&mtx, MTX_FREE) == MTX_LOCKED) {
                    return;
                }

                for (size_t i = 0; i < spin_n; ++i) {
                    if (mtx) {
                        if (cmpxchg(&mtx, MTX_LOCKED, MTX_LOCKED_AND_CONTESTED) != MTX_FREE) {
                            return;
                        }
                    }
                    cpu_relax();
                }
                // here 1 means wakeup one
                syscall(SYS_futex, &mtx, FUTEX_WAKE, 1, NULL, NULL, 0);
            }
        };
    }
}

#endif