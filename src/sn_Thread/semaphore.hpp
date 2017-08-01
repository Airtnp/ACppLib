#ifndef SN_THREAD_SEMAPHORE_H
#define SN_THREAD_SEMAPHORE_H

#include <mutex>
#include <condition_variable>
#include <cstddef>

namespace sn_Thread {
    // ref: http://blog.csdn.net/solstice/article/details/11432817
    // ref: https://stackoverflow.com/questions/4792449/c0x-has-no-semaphores-how-to-synchronize-threads
    namespace semaphore {
        // or template the mutex/cv type
        class SemaphoreBase /*: sn_Assist::sn_functional_base::noncopyable*/ {
        protected:
            SemaphoreBase() {}
            ~SemaphoreBase() {}
            std::mutex m_mutex;
            std::condition_variable m_cv;
            using naive_handle_type = typename std::condition_variable::naive_handle_type;
        };

        class Semaphore : private SemaphoreBase {
        public:

            explicit Semaphore(std::size_t count = 0) : m_count{count} {}
            Semaphore(const Semaphore&) = delete;
            Semaphore(Semaphore&&) = delete;
            Semaphore& operator=(const Semaphore) = delete;
            Semaphore& operator=(Semaphore&&) = delete;

            void wait() {
                std::unique_lock<std::mutex> lock{m_mutex};
                while(!m_count) {
                    m_cv.wait(lock);
                }
                --m_count;
            }
            // or broadcast
            void notify() {
                std::unique_lock<std::mutex> lock{m_mutex};
                ++m_count;
                m_cv.notify_one();
            }

            bool try_wait() {
                std::unique_lock<std::mutex> lock{m_mutex};
                if (m_count) {
                    --m_count;
                    return true;
                }
                return false;
            }

            template <typename Rep, typename Period>
            bool wait_for(const std::chrono::duration<Rep, Period>& d) {
                std::unique_lock<std::mutex> lock{m_mutex};
                auto is_finished = m_cv.wait_for(lock, d, [&]{ return m_count > 0; });
                if (is_finished) {
                    --m_count;
                }
                return is_finished;
            }

            template<class Clock, class Duration>
            bool wait_until(const std::chrono::time_point<Clock, Duration>& t) {
                std::unique_lock<std::mutex> lock{m_mutex};
                auto is_finished = m_cv.wait_until(lock, t, [&]{ return m_count > 0; });
                if (is_finished) {
                    --m_count;
                }
                return is_finished;
            }

            naive_handle_type naive_handle() {
                return m_cv.naive_handle();
            }

        private:
            std::size_t m_count;
        };
    }
}

#endif