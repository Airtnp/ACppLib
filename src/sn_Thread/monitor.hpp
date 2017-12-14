#ifndef SN_THREAD_MONITOR
#define SN_THREAD_MONITOR

#include <mutex>
#include <condition_variable>
#include <cstddef>

namespace sn_Thread {
    template <typename T>
    class Monitor {
    private:
        T m_obj;
        std::mutex m_mtx;
        std::condition_variable m_cv;
    public:
        Monitor(T&& obj) : m_obj{std::forward<T&&>(obj)} {}
        template <typename F>
        auto do_operation(F& fptr, Args&&... args) {
            std::unique_lock<std::mutex> guard{m_mtx};
            return (m_obj.*fptr)(std::forward<Args>(args)...);
        }
        template <typename F>
        auto do_operation(F& fptr, Args&&... args) const {
            std::unique_lock<std::mutex> guard{m_mtx};
            return (m_obj.*fptr)(std::forward<Args>(args)...);
        }
        void notify_one() {
            m_cv.notify_one();
        }
        void notify_all() {
            m_cv.notify_all();
        }
        void wait(std::unique_lock<std::mutex>& guard, P pred) {
            m_cv.wait(guard, pred);
        }
    };
}


#endif