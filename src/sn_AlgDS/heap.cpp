#ifndef SN_ALGDS_HEAP_H
#define SN_ALGDS_HEAP_H

#include <bits/stdc++.cpp>
using namespace std;

namespace heap {
    // left-child right sibling
    template <typename T>
    class LCRSBinaryTreeNode {
        LCRSBinaryTreeNode* m_parent = nullptr;
        LCRSBinaryTreeNode* m_prev = nullptr;
        LCRSBinaryTreeNode* m_left = nullptr; // child
        LCRSBinaryTreeNode* m_right = nullptr; // sibling;
        T m_value = T{};
    public:
        LCRSBinaryTreeNode() noexcept {}
        LCRSBinaryTreeNode(const T& v) noexcept : m_value{v} {}
        LCRSBinaryTreeNode(T&& v) noexcept : m_value{std::move(v)} {}
        
        LCRSBinaryTreeNode*& left_child() noexcept {
            return m_left;
        }

        LCRSBinaryTreeNode*& right_child() noexcept {
            return m_left->sibling();
        }

        LCRSBinaryTreeNode*& kth_child(size_t k) noexcept {
            TreeNode* res = m_left;
            while (k != 0 && res != nullptr) {
                res = res->sibling();
                --k;
            }
            return res;
        }

        LCRSBinaryTreeNode*& sibling() noexcept {
            return m_right;
        }

        LCRSBinaryTreeNode*& previous() noexcept {
            return m_prev;
        }

        LCRSBinaryTreeNode*& parent() noexcept {
            return m_parent;
        }
        
        T& value() noexcept {
            return m_value;
        }

        const T& value() noexcept {
            return m_value;
        }

    };

    template <typename T>
    struct MinPolicy {
        static bool compare(const T& l, const T& r) noexcept {
            return l < r;
        }
    };

    template <typename T>
    struct MaxPolicy {
        static bool compare(const T& l, const T& r) noexcept {
            return l > r;
        }
    };

    template <typename T, typename TN = LCRSBinaryTreeNode<T>, typename ComparePolicy = MinPolicy<T>>
    class BinaryHeap {
        TN* m_root = nullptr;
        TN* m_end = nullptr;
    public:
        BinaryHeap() {
            m_root = new TN;
            m_end = m_root;
        }
        
        void insert(const T& v) {
            TN* new_node = new TN{v};
            if (m_end->sibling() == nullptr) {
                m_end->sibling() = new_node;
                new_node->parent() = m_end->parent();
            } else {
                m_end->left_child() = new_node;
                new_node->parent() = m_end;
                m_end = ComparePolicy::compare(new_node->parent()->value(), new_node->value()) ? m_end : new_node ;
            }
            TN* n = new_node;
            TN* p = n->parent();
            while (p != nullptr && ComparePolicy::compare(p->value(), n->value())) {
                std::swap(p->value(), n->value());
                n = p;
                p = n->parent();
            }
        }

        // Only apply to !ComparePoly::compare(node->value(), v)
        void update(TN* node, const T& v) {
            node->value() = v;
            short sign = 0;
            if (node->left_child() != nullptr && ComparePolicy::compare(node->left_child()->value(), v)) {
                sign = 1;
            }
            if (node->right_child() != nullptr && ComparePolicy::compare(node->right_child()->value(), v)) {
                sign = 2;
            }
            if (sign == 0) return;
            if (sign == 1) {
                std::swap(node->value(), node->left_child->value());
                update(node->left_child(), v);
            } else {
                std::swap(node->value(), node->right_child->value());
                update(node->right_child(), v);
            }
        }

        T remove() {
            T res = m_root->left_child()->value();
            T upd;
            if (m_end->sibling() != nullptr) {
                upd = m_end->sibling()->value();
                delete m_end->sibling();
            } else {
                upd = m_end->value();
                m_end = m_end->parent();
                delete m_end->left_child();
            }
            update(m_root->left_child(), upd);
            return res;
        }

        void destroy(TN* node) {
            if (node == nullptr) return;
            if (node->left_child() != nullptr)
                destroy(node->left_child());
            if (node->right_child() != nullptr)
                destroy(node->right_child());
            delete node;
        }

        ~BinaryHeap() {
            destroy(m_root);
        }
    };

    template <typename T, typename TN = LCRSBinaryTreeNode<T>, typename ComparePolicy = MinPolicy<T>>
    class PairingHeap {
        TN* m_root = nullptr;
        size_t m_sz = 0;
    public:
        PairingHeap() : m_root{new TN} {}

        PairingHeap(const T& v) : m_root{new TN{v}} {}

        bool is_empty() noexcept {
            return m_sz == 0;
        }

        size_t size() noexcept {
            return m_sz;
        }

        T top() noexcept {
            return m_root->value();
        }

        void merge(TN* other_root) noexcept {
            T max = top();
            if (ComparePolicy::compare(max, other_root->top())) {
                TN* rest = m_root->left_child();
                other_root->sibling() = rest;
                m_root->left_child() = other_root;
            } else {
                TN* rest = other_root->left_child();
                m_root->sibling() = rest;
                other_root->left_child() = m_root;
                m_root = other_root;
            }
        }

        TN* merge(TN* root, TN* other_root) noexcept {
            T max = root->top();
            if (ComparePolicy::compare(max, other_root->top())) {
                TN* rest = root->left_child();
                other_root->sibling() = rest;
                root->left_child() = other_root;
            } else {
                TN* rest = other_root->left_child();
                root->sibling() = rest;
                other_root->left_child() = root;
                root = other_root;
            }
            return root;
        }

        void insert(const T& v) {
            TN* new_heap = new TN{v};
            merge(new_heap);
        }

        // If we have parent -> we can directly change parent left_child
        void update(TN* node, const T& v) {
            TN* new_heap = new TN{v};
            new_heap->left_child() = node->left_child();
            
            TN* new_left = node->sibling();
            // A copy
            node->value() = new_left->value();
            node->sibling() = new_left->sibling();
            node->left_child() = new_left->left_child();

            delete new_left;
            merge(new_heap);
        }

        T pop() {
            std::list<TN*> ns;
            TN* cur = m_root->left_child();
            while (cur != nullptr) {
                ns.push_back(cur);
                cur = cur->sibling();
            }
            T res = m_root->value();
            delete m_root;
            std::list<TN*> nns;
            while (ns.size() >= 2) {
                TN* tn1 = ns.pop_front();
                TN* tn2 = ns.pop_front();
                nns.push_back(merge(tn1, tn2));
            }
            if (ns.size() == 1)
                nns.push_back(ns.pop_front());
            while (nns.size() != 1) {
                TN* tn1 = nns.pop_back();
                TN* tn2 = nns.pop_back();
                nns.push_back(merge(tn1, tn2));
            }
            m_root = nns.pop_back();
            return res;
        }

        TN* pop(TN* root) {
            std::list<TN*> ns;
            TN* cur = root->left_child();
            while (cur != nullptr) {
                ns.push_back(cur);
                cur = cur->sibling();
            }
            T res = root->value();
            delete m_root;
            std::list<TN*> nns;
            while (ns.size() >= 2) {
                TN* tn1 = ns.pop_front();
                TN* tn2 = ns.pop_front();
                nns.push_back(merge(tn1, tn2));
            }
            if (ns.size() == 1)
                nns.push_back(ns.pop_front());
            while (nns.size() != 1) {
                TN* tn1 = nns.pop_back();
                TN* tn2 = nns.pop_back();
                nns.push_back(merge(tn1, tn2));
            }
            return nns.pop_back();
        }

        T remove(TN* node) {
            if (node == m_root) {
                return pop();
            } else {
                TN* new_heap = new TN{node->value()};
                new_heap->left_child() = node->left_child();
                
                TN* new_left = node->sibling();
                // A copy
                node->value() = new_left->value();
                node->sibling() = new_left->sibling();
                node->left_child() = new_left->left_child();
    
                delete new_left;

                TN* adjusted_new_heap = pop(new_heap);
                merge(adjusted_new_heap);
            }
        }

        void destroy(TN* node) {
            if (node == nullptr) return;
            if (node->left_child() != nullptr)
                destroy(node->left_child());
            if (node->sibling() != nullptr)
                destroy(node->sibling());
            delete node;
        }

        ~PairingHeap() {
            destroy(m_root);
        }
    };

    // Max-heap
    void adjust_heap(int arr[], size_t sz, size_t parent) {
        size_t left = idx * 2;
        size_t right = idx * 2 + 1;
        size_t large = parent;
        while (left < sz || right <sz) {
            if (left < sz && arr[parent] < arr[left]) {
                large = left;
            }
            if (right < sz && arr[parent] < arr[right]) {
                large = right;
            }
            if (large != parent) {
                std::swap(arr[parent], arr[large]);
                parent = large;
                left = parent * 2;
                right = parent * 2 + 1;
            } else {
                break;
            }
        }
    }

    void make_heap(int arr[], size_t sz) {
        size_t last_non_leaf = sz / 2 - 1;
        for (size_t i = last_non_leaf; i-- >0;) {
            adjust_heap(arr, sz, i);
        }
    }

    template <typename T>
    class MinMaxHeap {
        std::priority_queue<T, std::vector<T>, std::less<T>> m_less;
        // use std::unary_negate / decltype(std::not_fn(C)) before 17
        std::priority_queue<T, std::vector<T>, std::greater<T>> m_greater;
        size_t m_sz;
        T m_medianLarge;
        T m_medianLow;
    public:
        MinMaxHeap() : m_sz{0}, m_medianLarge{T{}}, m_medianLow{T{}} {}
    
        T median() {
            if (m_sz % 2)
                return m_medianLow;
            else
                return (m_medianLow + m_medianLarge) / 2;
        }

        bool empty() {
            return m_sz == 0;
        }
    
        size_t size() {
            return m_sz;
        }
    
        void insert(const T& value) {
            if (m_sz == 0) {
                m_medianLow = value;
                ++m_sz;
                return;
            }
            if (m_sz % 2) {
                if (value > this->median()) {
                    m_greater.push(value);
                    m_medianLarge = m_greater.top();
                    m_greater.pop();
                } else {
                    m_less.push(value);
                    m_medianLarge = m_medianLow;
                    m_medianLow = m_less.top();
                    m_less.pop();
                }
            } else {
                if (value >= m_medianLarge) {
                    m_greater.push(value);
                    m_less.push(m_medianLow);
                    m_medianLow = m_medianLarge;
                } else if (value <= m_medianLow) {
                    m_less.push(value);
                    m_greater.push(m_medianLarge);
                } else {
                    m_less.push(m_medianLow);
                    m_greater.push(m_medianLarge);
                    m_medianLow = value;
                }
            }
            ++m_sz;            
        }
    };
    
    template <typename T>
    struct SegTreeNode {
        bool is_value;
        T min;
        T max;
    };

    template <typename T, typename Comp = std::less<T>>
    class IntervalTree {
        std::vector<SegTreeNode<T>> m_data;
    public:
        IntervalTree(size_t n) {
            m_data.resize(n);
        }
        // arr must be ordered?
        void build(size_t root, T arr[], size_t beg, size_t end) {
            if (beg == end)
                m_data[root] = {true, arr[beg], arr[beg]};
            else {
                build(2 * root + 1, arr, beg, (beg + end) / 2);
                build(2 * root + 2, arr, (beg + end) / 2 + 1, end);
                m_data[root] = {
                    false,
                    std::min(m_data[2 * root + 1].min, m_data[2 * root + 2].min, Comp),
                    std::max(m_data[2 * root + 1].max, m_data[2 * root + 2].max, Comp)
                };
            }
        }
        void query(size_t root, const T& vmin, const T& vmax, std::vector<T>& vec) {
            if (Comp{}(m_data[root].max, vmin) || Comp{}(vmax, m_data[root])) {
                return;
            }
            if (root.is_value) {
                vec.push_back(root.min);
                return;
            }
            query(root * 2 + 1, vmin, vmax, vec);
            query(root * 2 + 2, vmin, vmax, vec);
        }
    };

    template <typename T, typename DT = T>
    struct ZKWHeapNode {
        T value;
        size_t L;
        size_t R;
        DT mark;
        ZKWHeapNode() {}
        ZKWHeapNode(const T& value_): value(value_) {}
        ZKWHeapNode(T&& value_): value(std::move(value_)) {}
    };

    template <typename T, typename Op>
    class ZKWHeap {
        using Node = ZKWHeapNode<T>;
        using Heap = ZKWHeap<T, Op>;
        using Lazy = size_t;
        static const constexpr init_lazy = 0;
        Node* m_nodeList;
        T* m_lazyList;
        size_t* m_L, m_R;
        size_t m_size;
        T init_v;
        
        void fix(size_t pos) {
            m_nodeList[pos] = Op{}(m_nodeList[pos << 1], m_nodeList[(pos << 1) + 1]);
        }

        template <typename U>
        void push_down(size_t x, const U& op) {
            if (m_lazyList[x] && x < m_sz) {
                m_lazyList[x << 1] += m_lazyList[x];    // redefine sum
                m_lazyList[(x << 1) + 1] += m_lazyList[x];
                m_nodeList[x << 1] = op(m_nodeList[x << 1], m_lazyList[x]);
                m_nodeList[(x << 1) + 1] = op(m_nodeList[(x << 1) + 1], m_lazyList[x]);
                m_lazyList[x] = init_lazy;
            }
        }

        template <typename U>
        void apply_lazy(size_t x, const U& op) {
            std::stack<int> s;
            while (x) {
                s.push(x);
                x >>= 1;
            }
            while (!s.empty()) {
                push_down(s.top(), op);
                s.pop();
            }
        }

        unsigned int log2(unsigned int x) {
            unsigned int ret;
            __asm__ __volatile__(
                "brsl %1, %%eax"
                :"=a"(ret)
                :"m"(x)
            );
            return ret;
        }
    public:
        ZKWHeap(size_t n, const T& init_v_) : init_v(init_v_) {
            m_sz = 1 << (1 + (size_t)(log2(n)));
            m_nodeList = new Node[m_sz << 1];
            m_lazyList = new T[m_sz << 1];
            for (size_t i = m_sz + 1; i <= m_sz + n; ++i) {
                m_nodeList[i].value = init_v;
                m_lazyList[i] = init_lazy;
                m_nodeList[i].L = i - M;
                m_nodeList[i].R = i - M; // Single point
            }
            for (size_t i = m_sz - 1; i > 0; --i) {
                // fix(i); // If we have initial operation
                m_nodeList[i].L = m_nodeList[i << 1].L;
                m_nodeList[i].R = m_nodeList[(i << 1) + 1].R;
                m_lazyList[i] = init_lazy;                
            }
        }

        ~ZKWHeap() {
            delete[] m_nodeList;
            delete[] m_lazyList;
        }

        const T& top() {
            return m_nodeList[1].value;
        }

        size_t top_pos() {
            return m_nodeList[1].mark;
        }

        void modify(size_t pos, const T& new_v) {
            int pos_ = pos + m_sz;
            m_nodeList[pos_].value = new_v;
            while(pos_) {
                fix(pos_ >>= 1);
            }
        }

        // For extreme, another way with fix up
        // ref: https://zhuanlan.zhihu.com/p/29937723
        template <typename U>
        void modify_range(size_t l, size_t r, const T& new_v, const U& op) {
            bool vl = false, vr = false; // 左右边第一个被访问的结点所在路径是否更新过
            int x;
            int sl, sr; // 记录左右两边第一个被访问的结点
            for (l = l + m_sz - 1, r = r + m_sz + 1; l ^ r ^ 1; l >>= 1, r >>= 1) {
                if (~l & 1) {
                    x = l ^ 1;
                    if (!vl) {
                        sl = x;
                        apply_lazy(x, op);
                        vl = true; // 将第一个被访问结点所在路径的Tag更新到结点中
                    }
                    m_lazyList[x] = op(m_lazyList[x], new_v);
                    m_nodeList[x] = op(m_nodeList[x], new_v);
                }
                if (r & 1) {
                    x = r ^ 1;
                    if (!vr) {
                        sr = x;
                        apply_lazy(x, op);
                        vr = true; // 将第一个被访问结点所在路径的Tag更新到结点中
                    }
                    m_lazyList[x] = op(m_lazyList[x], new_v);
                    m_nodeList[x] = op(m_nodeList[x], new_v);
                }
            }
            for (sl >>= 1; sl; sl >>= 1) {
                fix(sl);
            }
            for (sr >>= 1; sl; sr >>= 1) {
                fix(sr);
            }            
        }

        // lazy_operation
        template <typename U, typename S>
        void query_range(size_t l, size_t r, const U& op, S& sum) {
            bool vl = false, vr = false;
            // l ^ r ^ 1 == 0 => l, r is sibling
            for (l = l + m_sz - 1, r = r + m_sz + 1; l ^ r ^ 1; l >>= 1, r >>= 1) {
                if (~l & 1) {// L % 2 == 0 => l = lson(l / 2)
                    if (!vl) {
                        apply_lazy(l ^ 1, op);
                        vl = true;
                    }
                    sum(tree[l ^ 1]);
                }
                if (r & 1) { // R % 2 == 0 => r = rson(r / 2)
                    if (!vr) {
                        apply_lazy(r ^ 1, op);
                        vr = true;
                    }
                    sum(tree[r ^ 1]);
                }
            }
        }

        T pop() {
            T res = m_nodeList[1].value;
            modify(m_nodeList[1].mark, init_v);
            return res;
        }
    };
}



#endif