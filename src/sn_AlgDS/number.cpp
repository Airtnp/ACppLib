#ifndef SN_ALGDS_NUMBER_H
#define SN_ALGDS_NUMBER_H

#include <bits/stdc++.h>
using namespace std;


namespace number {
    
    int get_max(int* a,int l){
        // assert(l%4==0);
        // assert(sse4);
        int ret,tmp[4];
        __asm__ __volatile__ (
            "\txorps %%xmm0, %%xmm0\n"
            "LP4:\n"
            "\tpmaxsd (%1), %%xmm0\n"
            "\taddl $16, %1\n"
            "\tsubl $4, %2\n"
            "\tjnz LP4\n"
            "\tmovdqu %%xmm0, (%3)\n"
            "\tmovl (%3), %%eax\n"
            "\tcmpl 4(%3), %%eax\n"
            "\tcmovll 4(%3), %%eax\n"
            "\tcmpl 8(%3), %%eax\n"
            "\tcmovll 8(%3), %%eax\n"
            "\tcmpl 12(%3), %%eax\n"
            "\tcmovll 12(%3), %%eax\n"
            "\tmovl %%eax, %0\n"
            :"=m"(ret)
            :"r"(a),"r"(l),"r"(tmp)
            :"%eax"
        );
        return ret;
    }        

    namespace prime {

        constexpr size_t PRIME_LIMIT = 10000;

        bool is_prime(size_t n) {
            if (n <= 3) {
                return n > 1;
            } else if (n % 2 == 0 || n % 3 == 0) {
                return false;
            } else {
                size_t max_i = floor(sqrt(n));
                for (size_t i = 5; i < floor(sqrt(n)); i += 6) {
                    if (n % i == 0 || n % (i + 2) == 0)
                        return false;
                }
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

        vector<uint32_t> sundaram_prime_sieve(uint32_t n) {
            bitset<PRIME_LIMIT> sieve_arr{0};
            vector<uint32_t> prime;
            uint32_t m = (n - 2) / 2;
            // sieve all 2(i + j + 2ij) + 1 odd number (that's all odd non-prime)
            for (size_t i = 0; i < m; ++i) {
                uint32_t j = i;
                uint64_t p = (uint64_t)i + j + 2 * i * j;
                for (; p <= m; ++j, p = (uint64_t)i + j + 2 * i * j) {
                    sieve_arr[p] = 1;
                }
            }
            prime.reserve(m + 1);
            prime.push_back(2);
            for (size_t i = 1; i <= m; ++i) {
                if (!sieve_arr[i]) {
                    prime.push_back(2 * i + 1);
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

        // http://www.cnblogs.com/jackiesteed/articles/2019910.html
        unsigned int pollard_rho(unsigned int n) {
            unsigned int x = 2;
            unsigned int y = 2;
            unsigned int d = 1;
            auto f = [n](unsigned int v) { return (v ^ 2 + 1) % n; };
            while (d == 1) {
                x = f(x);
                y = f(x); // f(f(y))
                d = gcd(abs(x - y), n);
            }
            if (d == n)
                return 0;
            else
                return d;
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
        int solve_reverse_element_exgcd(int a, int n) {
            return solve_module_equation(a, 1, n);
        }

        int modpow(int base, int exp, int modulus) {
            base %= modulus;
            int result = 1;
            while (exp > 0) {
                if (exp & 1) {
                    result = (result * base) % modulus;
                    base = (base * base) % modulus);
                    exp >>= 1;
                }
            }
            return result;
        }

        // ax \equiv 1 (mod p), Fermat little theorem
        int solve_reverse_element_fermat(int a, int p) {
            return modpow(a, p - 2, p);
        }

        // prime index of n!
        int prime_index_factor(int n, int p) {
            int ans = 0;
            long long rec = p;
            while (n >= rec) {
                ans += n / rec;
                rec *= p;
            }
            return ans;
        }

        /// mod of combination number
        /// 1. C(n, m) = C(n - 1, m - 1) + C(n, m - 1)
        /// 2. Factorization. For each p,  the prime index pi(n) - pi(m) - pi(n - m) should contribute to the final result by power of p. Then do modpow
        /// 3. Lucas theorem. C(sp+r, up+v) \equiv C(s, u)C(r, v) (mod p)
        int mod_comb(int m, int n, int p) {
            // table of n! mod p
            vector<int> modtable(p + 1, 0);
            modtable[0] = 1;
            for (int i = 1; i <= m; ++i) {
                modtable[i] = ((long long)modtable[i - 1] * i) % p;
            }
            long long ans = 1;
            while (m || n) {
                int a = m % p;
                int b = n % p;
                m /= p;
                n /= p;
                int dividend = modtable[a];
                int divisor = (modtable[b] * modtable[a - b]) % p;
                int gcd = ::gcd(dividend, divisor);
                dividend /= gcd;
                divisor /= gcd;
                int rev_divisor = (solve_reverse_element_exgcd(divisor, p) + p) % p;
                ans = (ans * dividend * rev_divisor) % p;
            }
            return ans;
        }

        // # of numbers in 1..m which is not a coprime with factors.
        int notcoprimes(int m, const vector<int>& factors) {
            vector<int> ps;
            ps.push_back(-1);
            for (int f : factors) {
                int N = ps.size();
                for (int j = 0; j < N; ++j) {
                    ps.push_back(ps[j] * f * (-1));
                }
            }
            int sum = 0;
            for (int i = 1; i < ps.size(); ++i) {
                sum += m / ps[i];
            }
            return sum;
        }
    }

}

#endif