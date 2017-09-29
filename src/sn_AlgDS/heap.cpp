#ifndef SN_ALGDS_HEAP_H
#define SN_ALGDS_HEAP_H

#include <bits/stdc++.cpp>
using namespace std;

namespace heap {
    // left-child right sibling
    template <typename T>
    class LCRSBinaryTreeNode {
        TreeNode* m_parent = nullptr;
        TreeNode* m_left = nullptr; // child
        TreeNode* m_right = nullptr; // sibling;
        T m_value = T{};
    public:
        TreeNode() noexcept {}
        TreeNode(TreeNode* p, TreeNode* l, TreeNode* r, const T& v) noexcept :
            m_parent{p},
            m_left{l},
            m_right{r}, m_value{v} {}
        TreeNode(const T& v) noexcept : m_value{v} {}
        TreeNode(T&& v) noexcept : m_value{std::move(v)} {}
        
        TreeNode*& left_child() noexcept {
            return m_left;
        }

        TreeNode*& right_child() noexcept {
            return m_left->sibling();
        }

        TreeNode*& kth_child(size_t k) noexcept {
            TreeNode* res = m_left;
            while (k != 0 && res != nullptr) {
                res = res->sibling();
                --k;
            }
            return res;
        }

        TreeNode*& sibling() noexcept {
            return m_right;
        }

        TreeNode*& parent() noexcept {
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
}



#endif