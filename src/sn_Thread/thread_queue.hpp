#ifndef SN_THREAD_THREAD_QUEUE_h
#define SN_THREAD_THREAD_QUEUE_h

#include <thread>
#include <memory>
#include <mutex>


namespace sn_Thread {
    // @ref: https://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
    // @TODO: https://www.cs.rochester.edu/research/synchronization/pseudocode/queues.html
    namespace queue {
        // 单写单读
        // 多写多读 => cond wakes up random thread => 读写cond分开
        template <typename T>
        class ThreadQueue {
        public:
            T dequeue() {
                std::unique_lock<std::mutex> lock{m_mtx};
                while (m_queue.empty()) {
                    m_cond.wait(lock);
                }
                auto item = m_queue.front();
                m_queue.pop();
                return item;
            }

            void enqueue(const T& item) {
                std::unique_lock<std::mutex> lock{m_mtx}:
                m_queue.push(item);
                lock.unlock();
                m_cond.notify_one(); // 单写 + 保证不变量 ref: http://blog.csdn.net/ykxggg/article/details/19193081
            }
        private:
            std::queue<T> m_queue;
            std::mutex m_mtx;
            std::condition_variable m_cond;
        };
    }
}


#endif