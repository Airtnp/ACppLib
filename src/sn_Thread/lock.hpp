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
    }
}

#endif