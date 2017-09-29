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
        TreeNode() {}
        TreeNode(TreeNode* p, TreeNode* l, TreeNode* r, const T& v) :
            m_parent{p},
            m_left{l},
            m_right{r}, m_value{v} {}

        TreeNode* left_child() {
            return m_left;
        }

        TreeNode* right_child() {
            return m_left->sibling();
        }

        TreeNode* kth_child(size_t k) {
            TreeNode* res = m_left;
            while (k != 0 && res != nullptr) {
                res = res->sibling();
                --k;
            }
            return res;
        }

        TreeNode* sibling() {
            return m_right;
        }

        TreeNode* parent() {
            return m_parent;
        }
        
        T& value() {
            return m_value;
        }

        const T& value() {
            return m_value;
        }
    };

    template <typename T, typename TN = LCRSBinaryTreeNode<T>>
    class BinaryHeap {
        TN m_root;
    public:
        BinaryHeap() {}
        void insert() {

        }
    };
}



#endif