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
        size_t large = arr[parent];
        while (left < sz || right <sz) {
            if (left < sz && arr[parent] < arr[left]) {
                large = left;
            }
            if (right < sz && arr[parent] < arr[right]) {
                large = right;
            }
            if (large != arr[parent]) {
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
}



#endif