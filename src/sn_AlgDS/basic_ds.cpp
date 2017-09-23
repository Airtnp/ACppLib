#ifndef SN_ALGDS_BASIC_DS_H
#define SN_ALGDS_BASIC_DS_H

#include <bits/stdc++.h>
using namespace std;

namespace point {
    struct point2D {
        double x;
        double y;
    };

    struct point3D {
        double x;
        double y;
        double z;
    };

    double distance(point2D a, point2D b) {
        return ::sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
    }

    double distance(point3D a, point3D b) {
        return ::sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
    }

}

namespace basic {
    //˳��� ����/����/��չ/���������֣�/����/ɾ��
    template <typename T>
    class vector {
    private:
        int size, length;
        T *data;
    public:
        vector(int input_size) noexcept {
            size = input_size;
            length = 0;
            data = new T[size];
        }
        ~vector() {
            delete[] data;
        }
        bool insert(int loc, T value) {
            if (loc < 0 || loc > length) {
                return false;
            }
            if (length >= size) {
                return false;
            }
            for (int i = length; i > loc; --i) {
                data[i] = data[i - 1];
            }
            data[loc] = value;
            length++;
            return true;
        }
        int linear_search(T value) const {
            for (int i = 0; i < length; ++i) {
                if (data[i] == value) {
                    return i;
                }
            }
            return -1;
        }
        int bi_search(T value) const {
            int left = 0, right = length - 1;
            while (left <= right) {
                int mid = (left + right) / 2;
                if (data[mid] == value) {
                    return mid;
                }
                else if (data[mid] < value) {
                    left = mid + 1;
                }
                else {
                    right = mid - 1;
                }
            }
            return -1;
        }
        bool remove(int index) {
            if (index < 0 || index >= length) {
                return false;
            }
            for (int i = index + 1; i < length; ++i) {
                data[i - 1] = data[i];
            }
            length = length - 1;
            return true;
        }
        void print() const {
            for (int i = 0; i< length; i++) {
                if (i > 0) {
                    cout << " ";
                }
                cout << data[i];
            }
            cout << endl;
        }
    };

    //�������� ����/��ת/ɾ��
    template <typename T>
    class node {
    public:
        T data;
        node<T>* next;
        node(T _data) noexcept {
            data = _data;
            next = NULL;
        }
    };
    template <typename T>
    class linked_list {
    private:
        node<T>* head;
    public:
        linked_list() noexcept {
            head = NULL;
        }
        void insert(node<T> *node, int index) {
            if (head == NULL) {
                head = node;
                return;
            }
            if (index == 0) {
                node->next = head;
                head = node;
                return;
            }
            node<T> *current_node = head;
            int count = 0;
            while (current_node->next != NULL && count < index - 1) {
                current_node = current_node->next;
                count++;
            }
            if (count == index - 1) {
                node->next = current_node->next;
                current_node->next = node;
            }
        }
        void output() const {
            if (head == NULL) {
                return;
            }
            node<T> *current_node = head;
            while (current_node != NULL) {
                cout << current_node->data << " ";
                current_node = current_node->next;
            }
            cout << endl;
        }
        void delete_node(int index) {
            if (head == NULL) {
                return;
            }
            node<T> *current_node = head;
            int count = 0;
            if (index == 0) {
                head = head->next;
                delete current_node;
                return;
            }
            while (current_node->next != NULL && count < index - 1) {
                current_node = current_node->next;
                count++;
            }
            if (count == index - 1 && current_node->next != NULL) {
                node<T> *delete_node = current_node->next;
                current_node->next = delete_node->next;
                delete delete_node;
            }
        }
        void reverse() {
            if (head == NULL) {
                return;
            }
            node<T> *next_node, *current_node;
            current_node = head->next;
            head->next = NULL;
            while (current_node != NULL) {
                next_node = current_node->next;
                current_node->next = head;
                head = current_node;
                current_node = next_node;
            }
        }
    };

    //����: ����/ɾ��
    template <typename T>
    class queue {
    private:
        T *data;
        int head, tail, length, count;
    public:
        queue(int length_input) noexcept {
            data = new T[length_input];
            length = length_input;
            head = 0;
            tail = -1;
            count = 0;
        }
        ~queue() {
            delete[] data;
        }
        bool push(const T& element) {
            if (count < length) {
                tail = (tail + 1) % length;
                data[tail] = element;
                count++;
                return true;
            }
            else {
                return false;
            }
        }
        void output() const {
            for (int i = head; i != tail + 1; i = (i + 1) % length) {
                cout << data[i] << " ";
            }
            cout << endl;
        }
        int front() const {
            assert(count > 0);
            return data[head];
        }
        void pop() {
            assert(count > 0);
            head = (head + 1) % length;
            count--;
        }
        void empty() {
            return count == 0;
        }
        bool find(const T& elem) {
            for (int i = head; i != tail + 1; i = (i + 1) % length) {
                if (data[i] == elem)
                    return true;
            }
            return false;

        }

    };

    //ջ: ��ջ/��ջ
    template<class T> 
    class stack {
    private:
        T* data;
        int max_size, top_index;
    public:
        stack(int length_input) noexcept {
            data = new T[length_input];
            max_size = length_input;
            top_index = -1;
        }
        ~stack() {
            delete[] data;
        }
        bool push(const T &element) {
            if (top_index >= max_size - 1) {
                return false;
            }
            top_index++;
            data[top_index] = element;
            return true;
        }
        bool pop() {
            if (top_index < 0) {
                return false;
            }
            top_index--;
            return true;
        }
        T top() {
            assert(top_index >= 0);
            return data[top_index];
        }
        bool empty() const {
            if (top_index < 0) {
                return true;
            }
            else {
                return false;
            }
        }

    };

    //��ϣ��: ����/����/�ؽ�  -- use hash_policy and conflict resolution to rewrite
    class hash_table {
    private:
        string *elem;
        int size;
    public:
        hash_table(int size_): size(size_) {
            elem = new string[size];
            for (int i = 0; i < size; i++) {
                elem[i] = "#";
            }
        }

        ~hash_table() {
            delete[] elem;
        }
        int hash(string& index) {
            int code = 0;
            for (size_t i = 0; i < index.length(); i++) {
                code = (code * 256 + index[i] + 128) % size;
            }
            return code;
        }
        bool search(string& index, int& pos, int& times) {
            pos = hash(index);
            times = 0;
            while (elem[pos] != "#" && elem[pos] != index) {
                times++;
                if (times < size) {
                    pos = (pos + 1) % size;
                }
                else {
                    return false;
                }
            }
            if (elem[pos] == index) {
                return true;
            }
            else {
                return false;
            }
        }
        int insert(string& index) {
            int pos, times;
            if (search(index, pos, times)) {
                return 2;
            }
            else if (times < size / 2) {
                elem[pos] = index;
                return 1;
            }
            else {
                recreate();
                return 0;
            }
        }
        void recreate() {
            string* temp_elem;
            temp_elem = new string[size];
            for (int i = 0; i < size; i++) {
                temp_elem[i] = elem[i];
            }
            int copy_size = size;
            size *= 2;
            delete[] elem;
            elem = new string[size];
            for (int i = 0; i < size; i++) {
                elem[i] = "#";
            }
            for (int i = 0; i < copy_size; i++) {
                if (temp_elem[i] != "#") {
                    insert(temp_elem[i]);
                }
            }
            delete[] temp_elem;
        }

    };

    //������: ǰ��/����/����
    template <typename T>
    class bin_node {
    public:
        T data;
        bin_node *lchild, *rchild;
        bin_node(T _data) noexcept : data(_data), lchild(nullptr), rchild(nullptr) {}
        ~bin_node() {
            if (lchild != nullptr) {
                delete lchild;
            }
            if (rchild != nullptr) {
                delete rchild;
            }
        }
        void preorder() {
            cout << data << " ";
            if (lchild != nullptr) {
                lchild->preorder();
            }
            if (rchild != nullptr) {
                rchild->preorder();
            }
        }
        void inorder() {
            if (lchild != nullptr) {
                lchild->inorder();
            }
            cout << data << " ";
            if (rchild != nullptr) {
                rchild->inorder();
            }
        }
        void postorder() {
            if (lchild != nullptr) {
                lchild->postorder();
            }
            if (rchild != nullptr) {
                rchild->postorder();
            }
            cout << data << " ";
        }
    };

    template <typename T>
    class binary_tree {
    private:
        bin_node *root;
    public:
        binary_tree() {
            root = nullptr;
        }
        ~binary_tree() {
            if (root != nullptr) {
                delete root;
            }
        }
        struct child_option {
            enum select_child { lchild, rchild };
        };

        void insert(T value, bin_node<T> node, typename child_option::select_child op_child = child_option::lchild) {
            root->lchild = new bin_node<T>(value);
        }
        void preorder() {
            root->preorder();
        }
        void inorder() {
            root->inorder();
        }
        void postorder() {
            root->postorder();
        }
    };

    //����������
    template <typename T>
    class bin_sort_node {
    public:
        T data;
        bin_sort_node *lchild, *rchild, *father;
        bin_sort_node(T _data, bin_sort_node *_father = nullptr) :
            data(_data),
            lchild(nullptr),
            rchild(nullptr),
            father(_father) {}
        ~bin_sort_node() {
            if (lchild != nullptr) {
                delete lchild;
            }
            if (rchild != nullptr) {
                delete rchild;
            }
        }
        void insert(T value) {
            if (value == data) {
                return;
            }
            else if (value > data) {
                if (rchild == nullptr) {
                    rchild = new bin_sort_node<T>(value, this);
                }
                else {
                    rchild->insert(value);
                }
            }
            else {
                if (lchild == nullptr) {
                    lchild = new bin_sort_node<T>(value, this);
                }
                else {
                    lchild->insert(value);
                }
            }
        }
        bin_sort_node<T>* search(T value) {
            if (data == value) {
                return this;
            }
            else if (value > data) {
                if (rchild == nullptr) {
                    return nullptr;
                }
                else {
                    return rchild->search(value);
                }
            }
            else {
                if (lchild == nullptr) {
                    return nullptr;
                }
                else {
                    return lchild->search(value);
                }
            }
        }
        bin_sort_node<T>* predecessor() {
            bin_sort_node<T> *temp = lchild;
            while (temp != nullptr && temp->rchild != nullptr) {
                temp = temp->rchild;
            }
            return temp;
        }
        bin_sort_node<T>* successor() {
            bin_sort_node<T> *temp = rchild;
            while (temp != nullptr && temp->lchild != nullptr) {
                temp = temp->lchild;
            }
            return temp;
        }
        void remove_BinSortNode(bin_sort_node<T> *delete_BinSortNode) {
            bin_sort_node<T> *temp = nullptr;
            if (delete_BinSortNode->lchild != nullptr) {
                temp = delete_BinSortNode->lchild;
                temp->father = delete_BinSortNode->father;
                delete_BinSortNode->lchild = nullptr;
            }
            if (delete_BinSortNode->rchild != nullptr) {
                temp = delete_BinSortNode->rchild;
                temp->father = delete_BinSortNode->father;
                delete_BinSortNode->rchild = nullptr;
            }
            if (delete_BinSortNode->father->lchild == delete_BinSortNode) {
                delete_BinSortNode->father->lchild = temp;
            }
            else {
                delete_BinSortNode->father->rchild = temp;
            }
            delete delete_BinSortNode;
        }
        bool delete_tree(T value) {
            bin_sort_node<T> *delete_BinSortNode, *current_BinSortNode;
            current_BinSortNode = search(value);
            if (current_BinSortNode == nullptr) {
                return false;
            }
            if (current_BinSortNode->lchild != nullptr) {
                delete_BinSortNode = current_BinSortNode->predecessor();
            }
            else if (current_BinSortNode->rchild != nullptr) {
                delete_BinSortNode = current_BinSortNode->successor();
            }
            else {
                delete_BinSortNode = current_BinSortNode;
            }
            current_BinSortNode->data = delete_BinSortNode->data;
            remove_BinSortNode(delete_BinSortNode);
            return true;
        }
    };

    template <typename T>
    class binary_sort_tree {
    private:
        bin_sort_node<T> *root;
    public:
        binary_sort_tree() noexcept {
            root = nullptr;
        }
        ~binary_sort_tree() {
            if (root != nullptr) {
                delete root;
            }
        }
        void insert(T value) {
            if (root == nullptr) {
                root = new bin_sort_node<T>(value);
            }
            else {
                root->insert(value);
            }
        }
        bool find(T value) {
            if (root->search(value) == nullptr) {
                return false;
            }
            else {
                return true;
            }
        }
        bool delete_tree(T value) {
            return root->delete_tree(value);
        }
    };

    // 小根堆
    template <typename T>
    class heap {
    private:
        T *data;
        int size;
    public:
        heap(int length_input) {
            data = new T[length_input];
            size = 0;
        }
        ~heap() {
            delete[] data;
        }
        void push(T value) {
            data[size] = value;
            int current = size;
            int father = (current - 1) / 2;
            while (data[current] < data[father]) {
                swap(data[current], data[father]);
                current = father;
                father = (current - 1) / 2;
            }
            size++;
        }
        int top() {
            return data[0];
        }
        void update(int pos, int n) {
            int lchild = 2 * pos + 1, rchild = 2 * pos + 2;
            int max_value = pos;
            if (lchild < n && data[lchild] < data[max_value]) {
                max_value = lchild;
            }
            if (rchild < n && data[rchild] < data[max_value]) {
                max_value = rchild;
            }
            if (max_value != pos) {
                swap(data[pos], data[max_value]);
                update(max_value, n);
            }
        }
        void pop() {
            swap(data[0], data[size - 1]);
            size--;
            update(0, size);
        }
        int heap_size() {
            return size;
        }
    };

    // 并查集
    class disjoint_set {
    private:
        int *father, *rank;
    public:
        disjoint_set(int size) {
            father = new int[size];
            rank = new int[size];
            for (int i = 0; i < size; ++i) {
                father[i] = i;
                rank[i] = 0;
            }
        }
        ~disjoint_set() {
            delete[] father;
            delete[] rank;
        }
        int find_set(int node) {
            if (father[node] != node) {
                father[node] = find_set(father[node]);
            }
            return father[node];
        }
        bool merge(int node1, int node2) {
            int ancestor1 = find_set(node1);
            int ancestor2 = find_set(node2);
            if (ancestor1 != ancestor2) {
                if (rank[ancestor1] > rank[ancestor2]) {
                    swap(ancestor1, ancestor2);
                }
                father[ancestor1] = ancestor2;
                rank[ancestor2] = max(rank[ancestor1] + 1, rank[ancestor2]);
                return true;
            }
            return false;
        }
    };


    //T is like make_pair(vertex_index, some data) T(0) = null

    template <typename T>
    using graph_node = pair<int, T>;

    template <typename T>
    class matrix_edge {
    private:
        int size;
    public:
        vector<graph_node<T>> edge_data;
        matrix_edge(int size_) : size(size_) {
            for (int i = 0; i < size; ++i)
                edge_data.push_back(make_pair(0, 0));
        }
        graph_node<T>& operator[] (const int& index) {
            return edge_data[index];
        }
        const vector<graph_node<T>> get_adj() {
            vector<graph_node<T>> adj_v;
            for (const auto& v : edge_data) {
                adj_v.push_back(v);
            }
            return move(adj_v);
        }
    };

    template <typename T>
    class table_edge {
    private:
        int size;
    public:
        vector<graph_node<T>> edge_data;
        table_edge(int size_) : size(size_) {
        }
        graph_node<T>& operator[] (const int& index) {
            for (auto&& v : edge_data) {
                if (v.first == index)
                    return v;
            }
            return{ -1, 0 };
        }
        const vector<graph_node<T>> get_adj() {
            vector<graph_node<T>> adj_v;
            for (const auto& v : edge_data) {
                adj_v.push_back(v);
            }
            return move(adj_v);
        }
    };

    //T type, E edge_type
    template <typename T, typename E>
    class graph {
    public:
        const int size;
        vector<E> data;
        graph(int size_) : size(size_) {
            for (int i = 0; i < size; ++i)
                E me(size);
            data.push_back(move(me));
        }
        E& operator[] (const int& index) {
            return data[index];
        }
        const vector<graph_node<T>> get_adj(const int& index) {
            return move(data[index].get_adj());
        }
    };

    /*
    template <typename T>
    using matrix_graph = graph<T, matrix_edge<T>>;

    template <typename T>
    using table_graph = graph<T, table_edge<T>>;
    */

    template <typename T>
    class matrix_graph : public graph<T, matrix_edge<T>> {};

    template <typename T>
    class table_graph : public graph<T, table_edge<T>> {};

    class bitvec {
    public:
        bool* data;
        const int size;
        bitvec(int size_, bool dfl = false): size(size_) {
            data = new bool[size];
            memset(data, dfl, size);
        }
        ~bitvec() {
            delete[] data;
        }
        bool and_all() {
            for (int i = 0; i < size; ++i) {
                if (!data[i])
                    return false;
            }
            return true;
        }
    };


}



#endif