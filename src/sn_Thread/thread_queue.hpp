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
            T wait_and_pop() {
                std::unique_lock<std::mutex> lock{m_mtx};
                m_cond.wait(lock, [this]{
                    return !m_queue.empty();
                });
                auto item = std::move(*m_queue.front());
                m_queue.pop();
                return item;
            }

            void wait_and_pop(T& value) {
                std::unique_lock<std::mutex> lock{m_mtx};
                m_cond.wait(lock, [this]{
                    return !m_queue.empty();
                });
                value = std::move(*m_queue.front());
                m_queue.pop();
            }

            bool try_pop(T& value) {
                std::unique_lock<std::mutex> lock{m_mtx};
                if (m_queue.empty())
                    return false;
                value = std::move(*m_queue.front());
                m_queue.pop();
                return true;
            }

            void push(const T& item) {
                std::unique_lock<std::mutex> lock{m_mtx}:
                m_queue.push(item);
                m_cond.notify_one(); // 单写 + 保证不变量 ref: http://blog.csdn.net/ykxggg/article/details/19193081
            }
        private:
            std::queue<T> m_queue;
            std::mutex m_mtx;
            std::condition_variable m_cond;
        };

        template <typename T>
        class HeadTailQueue {
            struct Node {
                std::shared_ptr<T> data;
                std::unique_ptr<Node> next;
            };
            std::unique_ptr<Node> m_head;
            Node* m_tail;
            std::mutex m_head_mtx;
            std::mutex m_tail_mtx;
            std::condition_variable m_cond;

            Node* get_tail() {
                std::lock_guard<std::mutex> lk_tail{m_tail_mtx};
                return m_tail;
            }

            std::unique_ptr<Node> pop_head() {
                std::lock_guard<std::mutex> lk_head{m_head_mtx};
                if (m_head.get() == get_tail())
                    return nullptr;
                std::unique_ptr<Node*> old_head = std::move(m_head);
                m_head = std::move(old_head->next);
                return old_head;
            }

            std::unique_lock<std::mutex> wait_for_data() {
                std::unique_lock<std::mutex> lk_head{m_head_mtx};
                m_cond.wait(lk_head, [&]{
                    return m_head.get() != get_tail();
                });
                return std::move(lk_head);
            }

            std::unique_ptr<Node> wait_pop_head() {
                std::unique_lock<std::mutex> lk_head{wait_for_data()};
                return pop_head();
            }

            std::unique_ptr<Node> wait_pop_head(T& value) {
                std::unique_lock<std::mutex> lk_head{wait_for_data()};
                value = std::move(*m_head->data);
                return pop_head();
            }


        public:
            HeadTailQueue() : m_head{new Node}, m_tail{m_head.get()} {}
            HeadTailQueue(const HeadTailQueue&) = delete;
            HeadTailQueue& operator=(const HeadTailQueue&) = delete;
            
            std::shared_ptr<T> try_pop() {
                std::unique_ptr<Node> old_head = pop_head();
                return old_head ? old_head->data : std::shared_ptr<T>{};
            }

            std::shared_ptr<T> wait_and_pop() {
                const std::unique_ptr<Node> old_head = wait_pop_head();
                return old_head->data;
            }

            void empty() {
                std::lock_guard<std::mutex> lk_head{m_head_mtx};
                return m_head.get() == get_tail();
            }

            void wait_and_pop(T& value) {
                const std::unique_ptr<Node> old_head = wait_pop_head(value);
            }

            void push(T value) {
                std::shared_ptr<T> new_data = std::make_shared<T>(std::move(value));
                std::unique_ptr<Node> p(new Node);
                const Node* new_tail = p.get();
                {
                    std::lock_guard<std::mutex> lk_tail{m_tail_mtx};
                    m_tail->data = new_data;
                    m_tail->next = std::move(p);
                    m_tail = new_tail;
                }
                m_cond.notify_one();
            }
        };
    }
}


#endif