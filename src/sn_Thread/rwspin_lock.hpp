#ifndef SN_THREAD_RWSPIN_LOCK_H
#define SN_THREAD_RWSPIN_LOCK_H

#include <atomic>
#include <memory>
#include <thread>
#include <condition_variable>
#include <utility>

#if defined(__GNUC__)

#define likely(x) __builtin_expect ((x), 1) 
#define unlikely(x) __builtin_expect ((x), 0)

#else

#define likely(x) x
#define unlikely(x) x

#endif



namespace sn_Thread {

    // @Note: Actually C++14/17 has shared_mutex
    using std::mutex;
    using std::condition_variable;
    using std::unique_lock;
    using std::atomic;
    using std::memory_order;
    using std::lock_guard;
    using std::addressof;

    // @ref: https://stackoverflow.com/questions/41193648/c-shared-mutex-implementation
    class atomic_shared_mutex {
        atomic<uint32_t> m_refcnt{0};
        constexpr const static uint32_t STATE_WRITE = 1U << (sizeof(uint32_t) * 8 - 1);
    public:
        void lock() {
            uint32_t val;
            do {
                val = 0;
            } while (!m_refcnt.compare_exchange_weak(
                val, 
                STATE_WRITE,
                memory_order_acquire
            ));
        }

        void unlock() {
            m_refcnt.store(0, memory_order_release);
        }

        void lock_shared() {
            uint32_t val;
            do {
            while (val == STATE_WRITE) {
                val = m_refcnt.load(memory_order_relaxed);
            } 
            } while(!m_refcnt.compare_exchange_weak(
                val, 
                val + 1,
                memory_order_acquire
            ));
        }

        void unlock_shared() {
            m_refcnt.fetch_sub(1, memory_order_release);
        }
    };

    // @ref: https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/std/shared_mutex
    class shared_mutex {
        mutex m_mtx;
        // block when STATE_WRITE set or MAX_READER
        condition_variable m_blk_wt;
        // block when reader is non-zero
        condition_variable m_blk_rd;
        uint32_t m_cnt{0};
        constexpr const static uint32_t STATE_WRITE = 1U << (sizeof(uint32_t) * 8 - 1);
        constexpr const static uint32_t MAX_READER = ~STATE_WRITE;
        bool is_on_write() const {
            return m_cnt & STATE_WRITE;
        }

        bool num_reader() const {
            return m_cnt & MAX_READER;
        }

    public:
        void lock() {
            unique_lock<mutex> lk{m_mtx};
            // wait until getting writer lk
            m_blk_wt.wait(lk, [=]{ return !is_on_write(); });
            m_cnt |= STATE_WRITE;
            // wait until reader exits
            m_blk_rd.wait(lk, [=]{ return !num_reader(); });
        }

        void unlock() {
            lock_guard<mutex> lk{m_mtx};
            m_cnt = 0;
            m_blk_wt.notify_all();
        }

        void lock_shared() {
            unique_lock<mutex> lk{m_mtx};
            m_blk_wt.wait(lk, [=]{ return m_cnt < MAX_READER; });
            ++m_cnt;
        }

        void unlock_shared() {
            lock_guard<mutex> lk{m_mtx};
            auto prev = m_cnt--;
            if (is_on_write()) {
                // Wake on writer if no more reader
                if (!num_reader()) {
                    m_blk_rd.notify_one();
                }
            } else {
                // Wake on reader overflow
                if (prev == MAX_READER) {
                    m_blk_wt.notify_one();
                }
            }
        }
    };

    // @ref: https://code.woboq.org/llvm/libcxx/include/shared_mutex.html
    template <typename Mtx>
    class shared_lock {
        Mtx* m_mtx;
        bool is_own;
    public:
        explicit shared_lock(Mtx& mtx) noexcept : m_mtx(addressof(mtx)), is_own(true) {
            m_mtx->lock_shared();
        }
        shared_lock(const shared_lock&) = delete;
        shared_lock& operator=(const shared_lock&) = delete;
        shared_lock(shared_lock&& u) noexcept : m_mtx{u.m_mtx}, is_own{u.is_own} {    {
            u.m_mtx = nullptr;
            u.is_own = false;
        }
        shared_lock& operator=(shared_lock&& u) noexcept {
            if (is_own) {
                m_mtx->unlock_shared();
            }
            m_mtx = u.m_mtx;
            is_own = u.is_own;
            u.m_mtx = nullptr;
            u.is_own = false;
            return *this;
        }
        ~shared_lock() {
            if (is_own) {
                m_mtx->unlock_shared();
            }
        }
    };

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
            RWSpinLock* m_lock{};
    
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