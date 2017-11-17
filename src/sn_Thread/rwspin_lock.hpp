#ifndef SN_THREAD_RWSPIN_LOCK_H
#define SN_THREAD_RWSPIN_LOCK_H

#include <atomic>
#include <memory>
#include <thread>

#if defined(__GNUC__)

#define likely(x) __builtin_expect ((x), 1) 
#define unlikely(x) __builtin_expect ((x), 0)

#else

#define likely(x) x
#define unlikely(x) x

#endif



namespace sn_Thread {
    // ref: https://www.zhihu.com/question/67990738
    namespace rwspin_lock {
        class RWSpinLock {
            std::atomic<size_t> m_cnt{0};
            std::atomic<bool> m_need_write{false};

            friend class RWReadGuard;
            friend class RWWriteGuard;
        };

        class RWReadGuard {
        private:
            RWSpinLock* m_lock{};
        public:
            RWLockReadGuard() noexcept = default;
            
            explicit RWReadGuard(RWSpinLock * lock) noexcept : m_lock{lock} {
                while (true) {
                    if (unlikely(lock->m_need_write.load(std::memory_order_acquire))) {
                        std::this_thread::yield();
                        continue;
                    }
                    lock->m_cnt.fetch_add(1);
                    if (unlikely(lock->m_need_write.load(std::memory_order_acquire))) {
                        lock->m_cnt.fetch_sub(1);
                        continue;
                    }
                }
            }
    
            // @TODO
            RWReadGuard(RWReadGuard && another) = delete;
    
            // @TODO
            RWReadGuard & operator=(RWReadGuard && another) = delete;
    
            ~RWReadGuard() noexcept {
                if (m_lock) {
                    m_lock->m_cnt.fetch_sub(1);
                }
            }
    
        public:
            RWLockReadGuard(const RWLockReadGuard &) noexcept = delete;
    
            void operator=(const RWLockReadGuard &) noexcept = delete;
        };
    
        class RWWriteGuard {
        private:
            RWSpinLock * m_lock{};
    
        public:
            RWWriteGuard() noexcept = default;
    
            explicit RWWriteGuard(RWSpinLock * lock) noexcept : m_lock{lock} {
                while (true) {
                    if (unlikely(lock->m_need_write.load(std::memory_order_acquire))) {
                        std::this_thread::yield();
                        continue;
                    }
                    bool e = false;
                    if (unlikely(lock->m_need_write.compare_exchange_strong(e, true))) {
                        break;
                    }
                }
            }
    
            RWLockWriteGuard(RWLockWriteGuard && another) = delete;
    
            RWLockWriteGuard & operator=(RWLockWriteGuard && another) = delete;
    
            ~RWLockWriteGuard() noexcept {
                if (m_lock) {
                    m_lock->m_need_write.store(false, std::memory_order_release);
                }
            }
    
        public:
            RWLockWriteGuard(const RWLockWriteGuard &) noexcept = delete;
    
            void operator=(const RWLockWriteGuard &) noexcept = delete;
        };
    }
}

#endif