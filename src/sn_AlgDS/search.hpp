#ifndef SN_ALGDS_SEARCH_H
#define SN_ALGDS_SEARCH_H

#include <bits/stdc++.h>
using namespace std;

namespace search {
    
    template <typename C, typename T>
    int bisearch(C sorted_arr, T value, int l, int r) {
        while (r >= l) {
            int mid = (l + r) / 2;
            if (value == sorted_arr[mid])
                return mid;
            if (value < sorted_arr[mid])
                r = mid - 1;
            else
                l = mid + 1;
        }
        return -1;
    }

    //T support .next(), return loop length
    template <typename T>
    int floyd_judge_loop(const T& head_node, const T& end_node) {
        auto p1 = head_node;
        auto p2 = head_node;
        bool has_loop = false;
        while (p1 != end_node && p2 != end_node) {
            p1 = p1.next();
            p2 = p2.next().next();
            if (p1 == p2) { //The meeting node is the loop's first node, since the exceeded length L, loop length C, C | L
                has_loop = true;
                break;
            }
        }
        if (!has_loop)
            return -1;

        int loop_length = 0;
        do {
            loop_length++;
            p1 = p1.next();
        } while (p1 != p2);

        return loop_length;
    }

    //Max subarray sum
    int kadane(int arr[], int n) {
        int max_so_far = 0;
        int max_ending_here = 0;
        for (int i = 0; i < n; ++i) {
            max_ending_here = max_ending_here + arr[i];
            max_ending_here = std::max(max_ending_here, 0);
            max_so_far = std::max(max_so_far, max_ending_here);
        }
        return max_so_far;
    }

    //Max subarray product
    int kadane_product(int arr[], int n) {
        int max_so_far = 0;
        int max_ending_here = 0;
        int min_ending_here = 0;
        for (int i = 0; i < n; ++i) {
            int temp = max_ending_here;
            max_ending_here = std::max(arr[i], std::max(arr[i] * max_ending_here, arr[i]*min_ending_here));
            min_ending_here = std::min(arr[i], std::max(arr[i] * temp, arr[i] * min_ending_here));
            max_so_far = std::max(max_so_far, max_ending_here);
        }
        return max_so_far;
    }


}



#endif