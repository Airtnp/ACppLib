#ifndef SN_ALGDS_SORT_H
#define SN_ALGDS_SORT_H

#include <bits/stdc++.h>
using namespace std;

namespace sort {
    template <typename T>
    void insertion_sort(T arr[], size_t left, size_t right) {
        for (size_t i = right - 1; i > left; --i) {
            if (a[i - 1] > a[i]) {
                std::swap(a[i - 1], a[i]);
            }
        }

        for (size_t i = left + 2; i < right; ++i) {
            T v = a[i]; size_t j = i;
            while (v < a[j - 1]) {
                a[j] = a[j - 1];
                --j;
            }
            a[j] = v;
        }
    }

    template <typename T>
    void selection_sort(T arr[], size_t left, size_t right) {
        for (size_t i = left; i < right - 1; ++i) {
            size_t min = i;
            for (size_t j = i + 1; j < right; ++i) {
                if (a[j] < a[min])
                    min = j;
            }
            if (min != i) 
                std::swap(a[i], a[min]);
        }
    }

    template <typename T>
    void bubble_adaptive(T arr[], size_t left, size_t right) {
        for (size_t i = left; i < right - 1; ++i) {
            bool swapped = false;
            for (size_t j = right - 1; j > i; --j) {
                if (a[j] < a[j - 1]) {
                    swapped = true;
                    std::swap(a[j - 1], a[j]);
                }
            }
            if (!swapped)
                break;
        }
    }

    template <typename T>
    size_t partition(T arr[], size_t left, size_t right) {
        size_t pivot = --right;
        while (true) {
            while (a[left] < a[pivot] && left < right)
                ++left;
            while (a[right] >= a[pivot] && left < right)
                --right;
            if (left >= right)
                break;
            swap(a[left], a[right]);
        }
        swap(a[left], a[pivot]);
        return left;
    }

    template <typename T>
    void quick_sort(T arr[], size_t left, size_t right) {
        if (left >= right)
            return;
        size_t pivot = partition(arr, left, right);
        quick_sort(arr, left, pivot - 1);
        quick_sort(arr, pivot + 1, right);
    }

    // TODO: rotate + in place merge
    // ref: https://github.com/liuxinyu95/AlgoXY/blob/algoxy/sorting/merge-sort/src/mergesort.c
    // ref: linked-list-merge-sort
    template <typename T>
    void merge(T arr[], size_t left, size_t mid, size_t right) {
        size_t sz = right - left + 1;
        vector<T> c(sz);

        for (size_t i = left, j = mid + 1, k = 0; k < sz; ++k) {
            if (i > mid) {
                c[k] = a[j];
                ++j;
            } else if (j > right) {
                c[k] = a[i];
                ++i;
            } else {
                c[k] = (a[i] <= a[j]) ? a[i++] : a[j++];
            }
        }

        std::copy(c.begin(), c.end(), &a[left]);
    }

    // Top-down
    template <typename T>
    void merge_sort(T arr[], size_t left, size_t right) {
        if (right <= left)
            return;
        size_t mid = (right + left) / 2;
        merge_sort(arr, left, mid);
        merge_sort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }

    // bottom-up
    template <typename T>
    void merge_sort_bu(T arr[], size_t left, size_t right) {
        size_t dis = right - left;
        for (size_t sz = 1; sz < dis; sz *= 2) {
            for (size_t i = left; i < dis; i += sz * 2) {
                merge(arr, i, i + sz - 1, std::min(i + sz * 2 - 1, right));
            }
        }
    }
}


#endif