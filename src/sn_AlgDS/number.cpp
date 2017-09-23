#ifndef SN_ALGDS_NUMBER_H
#define SN_ALGDS_NUMBER_H

#include <bits/stdc++.h>
using namespace std;


namespace number {
    
    namespace prime {

        constexpr size_t PRIME_LIMIT = 10000;

        bool is_prime(unsigned int n) {
            for (size_t i = 2; i < floor(sqrt(n)); ++i) {
                if (i % n)
                    return false;
            }
            return true;
        }

        vector<unsigned int> linear_prime_sieve(unsigned int n) {
            bitset<PRIME_LIMIT> sieve_arr(0);
            vector<unsigned int> prime;
            for (size_t i = 2; i < n; ++i) {
                if (!sieve_arr[i])
                    prime.push_back(i);
                for (size_t j = 0; i * prime[j] < n; ++j) {
                    sieve_arr[i * prime[j]] = 1;
                    if (!(i % prime[j]))
                        break;
                }
            }
            return prime;
        }

        /*
        for detail: https://www.zhihu.com/question/29580448
                    http://cstheory.stackexchange.com/questions/5578/can-merlin-convince-arthur-about-a-certain-sum
        */
        unsigned int fast_prime_sum(unsigned int n) {
            unsigned int sqrt_n = floor(sqrt(n));
            unsigned int arr_index[PRIME_LIMIT * 10];  //contain the value of n/1, n/2, ... n / sqrt(n) , 1, ..., sqrt(n)
            unsigned int sum_prime[PRIME_LIMIT]; //contain the sum of first n primes
            for (unsigned int k = 0; k < sqrt_n + 1; ++k) {
                arr_index[k] = k;
                sum_prime[k] = (k * (k + 1)) / 2 - 1;
            }
            for (unsigned int k = sqrt_n + 1; k < 2 * sqrt_n + 1; ++k) {
                arr_index[k] = n / (2 * sqrt_n + 1 - k);
                sum_prime[k] = (arr_index[k] * (arr_index[k] + 1)) / 2 - 1;
            }
            for (unsigned int p = 2; p < sqrt_n + 1; ++p) {
                if (sum_prime[p] > sum_prime[p - 1]) {  //condition: p is a prime
                    unsigned int sum_pre = sum_prime[p - 1];
                    unsigned int square_p = p * p;
                    for (unsigned int q = 2 * sqrt_n; q > 1; --q) {
                        if (arr_index[q] < square_p)
                            break;
                        unsigned int a = arr_index[q];
                        unsigned int b = a / p;
                        a = a > sqrt_n ? 2 * sqrt_n + 1 - n / a : a; //map arr_index to indices of sum_prime
                        b = b > sqrt_n ? 2 * sqrt_n + 1 - n / b : b;
                        sum_prime[a] -= p * (sum_prime[b] - sum_pre);
                    }
                }
            }
            return sum_prime[2 * sqrt_n];
        }

    }

    namespace gcd {
        int gcd(int a, int b) {
            return b ? gcd(b, a % b) : a;
        }

        // ref: https://code.woboq.org/linux/linux/lib/gcd.c.html
        // ref: https://en.wikipedia.org/wiki/Binary_GCD_algorithm
        unsigned long binary_gcd(unsigned long a, unsigned long b) {
#if defined(AVAILABLE_FFS)
            unsigned long r = a | b;
            if (!a || !b)
                return r;
            b >>= __ffs(b);
            if (b == 1)
                return r & -r;
            for (;;) {
                a >>= __ffs(a);
                if (a == 1)
                    return r & -r;
                if (a == b)
                    return a << __ffs(r);
                if (a < b)
                    swap(a, b);
                a -= b;
            }
#else
            unsigned long r = a | b;
            if (!a || !b)
                return r;

            //get lowest bit of r
            r = sn_Alg::binary::lowbit(r);

            while (!(b & r))
                b >>= 1;
            if (b == r)
                return r;

            for (;;) {
                while (!(a & r))
                    a >>= 1;
                if (a == r)
                    return r;
                if (a == b)
                    return a;
                if (a < b)
                    swap(a, b);
                a -= b;
                a >>= 1;
                if (a & r)
                    a += b;
                a >>= 1;
            }
#endif
        }


        //return gcd(a, b) ax + by = gcd(a, b)
        int exgcd(int a, int b, int&x, int& y) {
            if (!b) {
                x = 1;
                y = 0;
                return a;
            }
            int r = exgcd(b, a % b, x, y);
            int t = x;
            x = y;
            y = t - a / b * y;
            return r;
        }

        //ax \\equiv b (mod n)
        int solve_module_equation(int a, int b, int n) {
            int x = 0, y = 0;
            int d = exgcd(a, n, x, y);
            if (b % d == 0) {
                return x * b / d;
            }
            return -1;
        }

        //ax \\equiv 1 (mod n)
        int solve_reverse_element(int a, int n) {
            return solve_module_equation(a, 1, n);
        }

    }

}

#endif