#ifndef SN_ALGDS_DP_H
#define SN_ALGDS_DP_H

#include <bits/stdc++.h>
using namespace std;


namespace dynamic_programming {
    namespace knapsack {
        //if T support </> we can apply to n-dimension
        template <typename T>
        T recursive_1d_knapsack(T capacity, const vector<T>& size, const vector<T>& value) {
            int sz = value.size();
            unordered_map<T, T> mt{};
            return recursive_1d_knapsack_helper(capacity, 0, size, value, mt);
        }

        template<typename T>
        T recursive_1d_knapsack_helper(T capacity, int index, const vector<T>& size, const vector<T>& value, unordered_map<T, T>& mt) {
            auto p = mt.find(capacity);
            if (p != mt.end())
                return p->second;
            // should return last item negative...
            if (capacity < 0)
                return 0;
            if (index == value.size())
                return 0;
            T res1 = recursive_1d_knapsack_helper(capacity, index + 1, size, value, mt);
            T res2 = recursive_1d_knapsack_helper(capacity - size[index], index + 1, size, value, mt) + value[index];
            T res = res1 > res2 ? res1 : res2;
            mt.insert({ capacity, res });
            return res;
        }

    }
}

#endif