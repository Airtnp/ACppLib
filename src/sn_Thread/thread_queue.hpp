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
                // prev unlock
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

        // @TODO: https://zhuanlan.zhihu.com/p/22557362
        // High-load problem
        template <typename T>
        class LockFreeStack {
            struct Node {
                std::shared_ptr<T> data;
                Node* next;
                Node(const T& data_) : data(data_) {}
            };

            static void delete_nodes(Node* node) {
                while (nodes) {
                    Node* n = nodes->next;
                    delete nodes;
                    nodes = n;
                }
            }

            void chain_pending_nodes(Node* nodes) {
                Node* last = nodes;
                while (const Node* n = last->next) {
                    last = next;
                }
                chain_pending_nodes(nodes, last);
            }

            void chain_pending_nodes(Node* first, Node* last) {
                last->next = m_to_be_deleted;
                while (!m_to_be_deleted.compare_exchange_weak(last->next, first));
            }

            void chain_pending_node(Node* head) {
                chain_pending_nodes(head, head);
            }

            void try_reclaim(Node* old_head) {
                if (m_thds_in_pop == 1) {
                    Node* nodes_to_delete = m_to_be_deleted.exchange(nullptr);
                    if (!--m_thds_in_pop) {
                        delete_nodes(nodes_to_delete);
                    } else if (nodes_to_delete) {
                        chain_pending_nodes(nodes_to_delete);
                    }
                    delete old_head;
                } else {
                    chain_pending_node(old_head);
                    --m_thds_in_pop;
                }
            }

            std::atomic<unsigned int32_t> m_thds_in_pop = 0;
            std::atomic<Node*> m_head;
            std::atomic<Node*> m_to_be_deleted;
        public:
            void push(const T& v) {
                const Node* new_node = new Node{v};
                new_node->next = m_head.load();
                while (!m_head.compare_exchange_weak(new_node->next, new_node));
            }

            std::shared_ptr<T> pop() {
                ++m_thds_in_pop();
                Node* old_head = m_head.load();
                while (old_head && !m_head.compare_exchange_weak(old_head, old_head->next));
                std::shared_ptr<T> res;
                if (old_head) {
                    res.swap(old_head->data);
                }
                try_reclaim(old_head);
                return res;
            }
        };

        // From C++ Concurrency in Action
        template <typename T>
        class LockFreeQueue
        {
        private:
            struct node;
            struct counted_node_ptr {
                int external_count;
                node* ptr;
            };

            std::atomic<counted_node_ptr> head;
            std::atomic<counted_node_ptr> tail;

            struct node_counter {
                unsigned internal_count:30;
                unsigned external_counters:2;
                    // you need only 2 bits because there are at most two such
                    // counters (next and tail)
            };

            struct node {
                std::atomic<T*> data;
                std::atomic<node_counter> count;
                std::atomic<counted_node_ptr> next;

                node() {
                    node_counter new_count;
                    new_count.internal_count = 0;
                    new_count.external_counters = 2; 
                        // because every node starts out referenced from tail and
                        // from the next pointer of the previous node once you've
                        // actually aadded it to the queue
                    count.store(new_count);

                    counted_node_ptr new_next;
                    new_next.ptr = nullptr;
                    new_next.external_count = 0;
                    next.store(new_next);
                } 

                void release_ref() {
                    node_counter old_counter =  count.load(std::memory_order_relaxed);
                    node_counter new_counter;
                    do {
                        new_counter=old_counter;
                        --new_counter.internal_count;
                    } while(!count.compare_exchange_strong(
                                old_counter,new_counter,
                                std::memory_order_acquire,std::memory_order_relaxed));
                    if(!new_counter.internal_count && !new_counter.external_counters) {
                        delete this;
                    }
                }

            };

            void set_new_tail(counted_node_ptr &old_tail,
                            counted_node_ptr const &new_tail) {
                node* const current_tail_ptr = old_tail.ptr;
                while(!tail.compare_exchange_weak(old_tail,new_tail) &&
                    old_tail.ptr == current_tail_ptr);
                if(old_tail.ptr == current_tail_ptr)
                    free_external_counter(old_tail);
                else
                    current_tail_ptr->release_ref();
            }

            static void increase_external_count(
                std::atomic<counted_node_ptr>& counter,
                counted_node_ptr& old_counter) {
                counted_node_ptr new_counter;
                do {
                    new_counter = old_counter;
                    ++new_counter.external_count;
                } while(!counter.compare_exchange_strong(
                        old_counter,new_counter,
                        std::memory_order_acquire,std::memory_order_relaxed));
                old_counter.external_count = new_counter.external_count;
            }

            static void free_external_counter(counted_node_ptr &old_node_ptr)
            {
                node* const ptr = old_node_ptr.ptr;
                int const count_increase = old_node_ptr.external_count-2;
                node_counter old_counter =
                    ptr->count.load(std::memory_order_relaxed);
                node_counter new_counter;
                do {
                    new_counter=old_counter;
                    --new_counter.external_counters;
                    new_counter.internal_count+=count_increase;
                } while(!ptr->count.compare_exchange_strong(
                        old_counter,
                        new_counter,
                        std::memory_order_acquire,std::memory_order_relaxed));
                if(!new_counter.internal_count && !new_counter.external_counters) {
                    delete ptr;
                }
            }

        public:
            LockFreeQueue()
            : head()
            , tail()
            {}

            LockFreeQueue(const LockFreeQueue& other) = delete;
            LockFreeQueue& operator=(const LockFreeQueue& other) = delete;

            ~LockFreeQueue() {
                /*
                while (node* const old_head = head.load()) {
                    head.store(old_head->next);
                    delete old_head;
                }
                */
            }

            void push(T new_value) {
                std::unique_ptr<T> new_data(new T(new_value));
                counted_node_ptr new_next;
                new_next.ptr = new node;
                new_next.external_count=1;
                counted_node_ptr old_tail=tail.load();
                for(;;) {
                    increase_external_count(tail,old_tail);
                    T* old_data=nullptr;
                    if(old_tail.ptr->data.compare_exchange_strong(
                        old_data,new_data.get())) {
                        counted_node_ptr old_next={0};
                        if(!old_tail.ptr->next.compare_exchange_strong(
                            old_next,new_next)) {
                            delete new_next.ptr;
                            new_next=old_next;
                        }
                        set_new_tail(old_tail, new_next);
                        new_data.release();
                        break;
                    } else {
                        counted_node_ptr old_next={0};
                        if(old_tail.ptr->next.compare_exchange_strong(
                            old_next,new_next)) {
                            old_next=new_next;
                            new_next.ptr=new node;
                        }
                        set_new_tail(old_tail, old_next);
                    }
                }
            }

            std::unique_ptr<T> pop() {
                counted_node_ptr old_head = head.load(std::memory_order_relaxed);

                for(;;) {
                    increase_external_count(head, old_head);

                    node* const ptr = old_head.ptr;
                    if (ptr==tail.load().ptr) {
                        ptr->release_ref();
                        return std::unique_ptr<T>();
                    }

                    counted_node_ptr next = ptr->next.load();

                    if (head.compare_exchange_strong(old_head, next)) {
                        T* const res=ptr->data.exchange(nullptr);
                        free_external_counter(old_head);
                        return std::unique_ptr<T>(res);
                    }
                    ptr->release_ref();
                }
            }
        };

    }
}


#endif