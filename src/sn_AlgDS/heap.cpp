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

    
    template <typename T>
    struct FiboNode {
        using Node = FiboNode<T>;
        T value;
        size_t degree;
        Node* left, right, child, parent;
        bool is_marked;
        FiboNode(const T& value_)
            : value{value_}, degree{0},
            is_marked{false}, left{nullptr},
            right{nullptr}, child{nullptr}, parent{nullptr} {
            left = this;
            right = this;
        }
    };

    template <typename T, typename Comp = std::less<T>>
    class FiboHeap {
    public:
        using FN = FiboNode<T>;
        // FiboHeap() : m_sz{0}, m_maxdeg{0} {}
        FiboHeap() : m_heap{nullptr} {}
        ~FiboHeap() {
            if (m_heap) {
                delete_tree(m_heap);
            }
        }

        FN* insert(const T& value_) {
            FN* res = FN{value_};
            m_heap = merge(m_heap, res);
            return res;
        }

        T get_min() {
            return m_heap->value;
        }

        T extract_min() {
            FN* old = m_heap;
            m_heap = remove_min(m_heap);
            T res = old->value;
            delete old;
            return res;
        }

        void decrease_key(FN* node, const T& value) {
            m_heap = decrease_node(m_heap, node, value);
        }

        /*
        void combine(FiboHeap<T>* rhs) {
            if (rhs == nullptr)
                return;
            if (rhs->m_maxdeg > m_maxdeg)
                std::swap(*this, *rhs);
            if (m_min == nullptr) {
                m_min = rhs->m_min;
                m_sz = rhs->m_sz;
                delete rhs; // maybe rhs should still here
            } else if (rhs->m_min == nullptr) {
                delete rhs;
            } else {
                cat_list(m_min, rhs->m_min);
                if (Comp{}(rhs->m_min->value, m_min->value)) {
                    m_min = rhs->m_min;
                }
                m_sz += rhs->m_sz;
                delete rhs;
            }
        }

        FN* extract_min() {
            FN* p = m_min;
            if (p == p->right) {
                m_min = nullptr;
            } else {
                remove_node(p);
                m_min = p->right;
            }
            p->left = p->right = p;
            return p;
        }
        */

    private:

        FN* merge(FN* lhs, FN* rhs) {
            if (!lhs) return rhs;
            if (!rhs) return lhs;
            if (Comp{}(rhs->value, lhs->value)) {
                std::swap(lhs, rhs);
            }
            FN* lhs_next = lhs->right;
            FN* rhs_prev = rhs->left;
            lhs->right = rhs;
            rhs->left = lhs;
            lhs_next->left = rhs;
            rhs_prev->right = lhs;
            return lhs;
        }

        void delete_tree(FN* node) {
            if (node == nullptr)
                return;
            FN* p = node;
            do {
                FN* prev = p;
                p = p->right;
                delete_tree(prev->child);
                delete prev;
            } while (p != node);
        }

        void add_child(FN* parent, FN* child) {
            child->left = child->right = child;
            child->parent = parent;
            ++parent->degree;
            parent->child = merge(parent->child, child);
        }

        void unmark_all(FN* node) {
            if (node == nullptr)
                return;
            FN* p = node;
            do {
                p->is_marked = false;
                p->parent = nullptr;
                p = p->right;
            } while (p != node);
        }

        FN* remove_min(FN* node) {
            unmark_all(node->child);
            if (node->right == node)
                node = node->child;
            else {
                node->right->left = node->left;
                node->left->right = node->right;
                node = merge(node->right, node->child);
            }
            if (node == nullptr)
                return nullptr;
            std::vector<FN*> trees;
            trees.resize(64, nullptr);
            while (true) {
                if (trees[node->degree] != nullptr) {
                    FN* t = trees[node->degree];
                    if (t == node) break;
                    trees[node->degree] = nullptr;
                    t->left->right = t->right;
                    t->right->left = t->left;    
                    if (Comp{}(node->value, t->value)) {
                        add_child(node, t);
                    } else {
                        if (node->right == node) {
                            t->right = t->left = t;
                            add_child(t, node);
                            node = t;
                        } else {
                            node->left->right = t;
                            node->right->left = t;
                            t->right = node->right;
                            t->left = node->left;
                            add_node(t, node);
                            node = t;
                        }
                    }
                    continue;
                } else {
                    trees[node->degree] = node;
                }
                node = node->right;
            }
            FN* min = node;
            FN* pn = node;
            do {
                if (Comp{}(pn->value, min->value)) {
                    min = pn;
                }
                pn = pn->right;
            } while(pn != node);
            return min;
        }

        FN* cut(FN* heap, FN* node) {
            if (node->right == node) {
                node->parent->child = nullptr;
            } else {
                node->left->right = node->right;
                node->right->left = node->left;
                node->parent->child = node->right;
            }
            node->right = node->left = node;
            node->is_marked = false;
            return merge(heap, node);
        }

        FN* decrease_node(FN* heap, FN* node, const T& value) {
            if (Comp{}(node->value, value)) return heap;
            node->value = value;
            if (Comp{}(node->value, node->parent->value)) {
                heap = cut(heap, node);
                FN* parent = node->parent;
                node->parent = nullptr;
                while (parent != nullptr && parent->is_marked) {
                    heap = cut(heap, parent);
                    node = parent;
                    parent = node->parent;
                    node->parent = nullptr;
                }
                if (parent != nullptr && parent->parent != nullptr) {
                    parent->is_marked = true;
                }
            }
            return heap;
        }

        /*
        void add_node(FN* node, FN* root) {
            node->left = root->left;
            root->left->right = node;
            node->right = root;
            root->left = node;
        }

        void release_node(FN* node) {
            if (x != nullptr) {
                while (x->child != nullptr) {
                    FN* tmp = extract_node(x->child);
                    release_node(tmp);
                }
                delete x;
            }
        }

        void cat_list(FN* lhs, FN* rhs) {
            FN* tmp = lhs->right;
            lhs->right = rhs->right;
            rhs->right->left = lhs;
            rhs->right = tmp;
            tmp->left = rhs;
        }

        void insert_to_list(FiboNode* pos, FiboNode* pn) {
            if (pos != nullptr) {
                pos->right->left = pn;
                pn->right = pos->right;
                pos->right = pn;
                pn->left = pos;
                pn->parent = pos->parent;
                if (pos->parent != nullptr)
                    ++pos->parent->degree;
            }
        }

        FiboNode* extract_node(FiboNode* pn) {
            if (pn == nullptr)
                return nullptr;
            if (pn->parent != nullptr) {
                pn->is_marked = false;
                if (pn->right != pn) {
                    pn->parent->child = pn->right;
                } else {
                    pn->parent->child = nullptr;
                }
                if (pn->parent->parent != nullptr)
                    pn->parent->is_marked = true;
                --pn->parent->degree;
                pn->parent = nullptr;
            }
            if (pn->left != pn) {
                pn->left->right = pn->right;
                pn->right->left = pn->left;
                pn->left = pn;
                pn->right = pn;
            }
            pn->is_marked = false;
            return pn;
        }

        void consolidate() {
            if (m_min == nullptr)
                return;
            size_t max = static_cast<size_t>(log2(static_cast<float>(m_sz)));
            
        }*/

        FN* m_heap;
        /*
        size_t m_sz;
        size_t m_maxdeg;
        FiboNode<T>* m_min = nullptr;
        std::vector<FiboNode<T>*> m_cons;
        */
    };

    class AVL {
    public:
        struct Node {
            int datum;
            int height;
            Node* left;
            Node* right;
            int left_height() {
                return left ? left->height : 0;
            }
            int right_height() {
                return right ? right->height : 0;
            }
            int balance() {
                return left_height() - right_height();
            }
            // when the height of its children change, call this function to
            // recalculate the height of this node, the parent
            void fix_height() {
                height = 1 + max(left_height(), right_height());
            }
        };
        void insert(int datum);
        const Node* search(int datum, bool print_path = false) const;
        ~AVL();

        // a debugging method that will print the tree for you
        // (use it to inspect small trees, if you want)
        void debug_print_tree() {
            debug_print_node(0, root);
        }

        // NOTE: this function is really complicated
        // don't worry too much about how it works
        // TODO: support 3+ digit numbers
        void print_diagram() {
            struct pos {
                int depth;
                int parent_dir;
            };
            std::vector<std::pair<int, pos>> points;
            std::function<void(Node*, pos p)> traverse_depth = [&](Node* n, pos p) {
                if (n == nullptr) {
                    return;
                }
                traverse_depth(n->left, {p.depth+1, 1});
                points.push_back(std::pair<int, pos>({n->datum, p}));
                traverse_depth(n->right, {p.depth+1, -1});
            };
            traverse_depth(root, {0, 0});
            // points is now filled
            int width = 2 * (int)points.size();
            int height = 0;
            for (int i = 0; i < (int)points.size(); i++) {
                height = max(height, points[i].second.depth);
            }
            height *= 2;
            height++;
            // now, we can build the buffer:
            std::vector<std::vector<char>> buffer(width, std::vector<char>(height, ' '));
            // add the numbers
            for (int i = 0; i < (int)points.size(); i++) {
                int n = points[i].first;
                int d = points[i].second.depth;
                buffer[2*i+1][d*2] = char((n % 10) + '0');
                if (n >= 10) {
                    // note: will truncate 3+ digit numbers to their last 2 digits
                    buffer[2*i+0][d*2] = char((n / 10) % 10 + '0');
                }
            }
            // add the corner edges
            for (int i = 0; i < (int)points.size(); i++) {
                int d = points[i].second.depth;
                int dir = points[i].second.parent_dir;
                if (dir == 0) {
                    continue; // root
                }
                if (points[i + dir].second.depth == d-1) {
                    // adjacent parent
                    buffer[2*i + (dir > 0 ? 2 : 0)][d*2-1] = (dir > 0 ? '/' : '\\');
                } else {
                    int c = 2*i + (dir > 0 ? 2 : -1);
                    buffer[c][d*2-1] = (dir > 0 ? '/' : '\\');
                    buffer[c + dir][d*2-2] = '-';
                    for (int j = i+2*dir; points[j].second.depth != d-1; j += dir) {
                        buffer[2*j][d*2-2] = '-';
                        buffer[2*j+1][d*2-2] = '-';
                    }

                }
            }
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    std::cout << buffer[x][y];
                }
                std::cout << std::endl;
            }
        }
    private:
        Node* root = nullptr;
        size_t size = 0;

        // insert_node returns the increase in height for the provided tree
        Node* insert_node(Node* node, int datum);

        // search_node returns the node, or nullptr
        const Node* search_node(const Node* node, int datum, bool print_path) const;

        // these return the new top node after rotation
        Node* rotate_left(Node* node);
        Node* rotate_right(Node* node);
        void destroy_node(Node* node);

        void debug_print_node(int depth, Node* node) {
            if (node == nullptr) {
                return;
            }
            debug_print_node(depth+1, node->left);
            for (int i = 0; i < depth; i++) {
                std::cout << "  ";
            }
            std::cout << node->datum << std::endl;
            debug_print_node(depth+1, node->right);
        }

    };

    const AVL::Node* AVL::search(int datum, bool print_path) const {
        // just a regular BST search
        return search_node(root, datum, print_path);
    }

    // search_node searches for 'datum' in the subtree rooted at 'node'.
    // if the node cannot be found, it returns nullptr.
    const AVL::Node* AVL::search_node(const AVL::Node* node, int datum, bool print_path) const {
        if (node == nullptr) {
            if (print_path) {
                std::cout << "M" << std::endl;
            }
            return nullptr; // not found (no node here)
        }
        if (node->datum == datum) {
            // found the target
            if (print_path) {
                std::cout << "X" << std::endl;
            }
            return node;
        }
        if (datum < node->datum) {
            // left subtree, since smaller than current node
            if (print_path) {
                std::cout << "L";
            }
            return search_node(node->left, datum, print_path);
        } else {
            if (print_path) {
                std::cout << "R";
            }
            // right subtree, since larger than current node
            return search_node(node->right, datum, print_path);
        }
    }

    void AVL::insert(int datum) {
        root = insert_node(root, datum);
    }

    // insert_node returns the new root of this subtree after inserting datum.
    AVL::Node* AVL::insert_node(AVL::Node* node, int datum) {
        if (node == nullptr) {
            // at a leaf position in the tree, so create a new node
            return new Node { datum, 1, nullptr, nullptr }; // it has height 1
        }
        if (datum < node->datum) {
            node->left = insert_node(node->left, datum);
            node->fix_height(); // remember to fix the height of a node after modifying its children
            if (node->balance() > 1) {
                if (node->left->balance() < 0) {
                    node->left = rotate_left(node->left);
                }
                node = rotate_right(node);
            }
        } else {
            node->right = insert_node(node->right, datum);
            node->fix_height(); // remember to fix the height of a node after modifying its children
            if (node->balance() < -1) {
                if (node->right->balance() > 0) {
                    node->right = rotate_right(node->right);
                }
                node = rotate_left(node);
            }
        }
        return node;
    }

    // rotate_left performs a left rotation; it returns the new 'root' of the rotated subtree
    // (remember to update the heights of nodes!)
    // you may assume that it has a right child
    AVL::Node* AVL::rotate_left(AVL::Node* node) {
        using node_t = AVL::Node*;
        node_t pivot = node->right;
        node->right = pivot->left;
        pivot->left = node;
        node->fix_height();    
        pivot->fix_height();
        return pivot;
    }

    // rotate_right performs a right rotation; it returns the new 'root' of the rotated subtree
    // (remember to update the heights of nodes!)
    // you may assume that it has a left child
    AVL::Node* AVL::rotate_right(AVL::Node* node) {
        using node_t = AVL::Node*;
        node_t pivot = node->left;
        node->left = pivot->right;
        pivot->right = node;
        node->fix_height();    
        pivot->fix_height();
        return pivot;
    }

    AVL::~AVL() {
        destroy_node(root);
    }

    void AVL::destroy_node(AVL::Node* node) {
        if (node == nullptr) {
            return;
        }
        destroy_node(node->left);
        destroy_node(node->right);
        delete node;
    }

}



#endif