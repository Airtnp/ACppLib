#ifndef SN_ALGDS_MISCS_H
#define SN_ALGDS_MISCS_H

#include <bits/stdc++.h>
using namespace std;


namespace misc {

    //ref: https://en.wikipedia.org/wiki/Fast_inverse_square_root
    float q_rsqrt(float x) {
        float x2 = x * 0.5F;
        int i = *(int*) & x;
        i = 0x5f3759df - (i >> 1);    
        x = *(float*) & i;
        x = x * (1.5F - (x2 * x * x)); 
        return x;
    }

    unsigned long long get_clock_cycle() {
        unsigned long long ret, tickl, tickh;
#if defined (__GNUC__)

#if defined (SN_CONFIG_OS_i386)
        __asm__ __volatile__("rdtsc\n\t": "=A" (ret) : );
#elif defined (SN_CONFIG_OS_X86_64)
        __asm__ __volatile__("rdtsc\n\t": "=a"(tickl), "=d"(tickh));
        ret = (static_cast<unsigned long long>(tickh) << 32) | tickl
#endif

#else
        ret = __rdtsc();
        /*For another option : untested, better specify bits
        unsigned long ret_p;
        unsigned long ret_l;
        __asm {
            rdtsc;
            out edx, ret_p
            out eax, ret_l
        }
        ret = ret_p << 32 + ret_l
        */
#endif
        return ret;
    }

    inline int inc_mod(int x, int v, int m) {
        x += v;
        return x >= m ? x - m : x;
    }

    inline int dec_mod(int x, int v, int m) {
        x -= v;
        return x >= m ? x - m : x;
    }

    inline char getchar_adj() {
#if defined(__POSIX__)
        return static_cast<char>(getchar_unlocked());
#else
        return static_cast<char>(getchar());
#endif
    }

    inline void putchar_adj(char c) {
#if defined(__POSIX__)
        putchar_unlocked(c);
#else
        putchar(c);
#endif
    }
        
    inline int fread_adj(void *data, size_t size, size_t count, FILE *stream) {
#if defined(__POSIX__)
        return fread_unlocked(data, size, count, stream);
#else
        return fread(data, size, count, stream);
#endif
    }

    constexpr const static size_t SN_ALG_FREAD_BUFFER_SIZE = 1000;
    char sn_alg_fread_buf[SN_ALG_FREAD_BUFFER_SIZE];
    char* sn_alg_fread_s = sn_alg_fread_buf + SN_ALG_FREAD_BUFFER_SIZE;
    size_t sn_alg_fread_sz = SN_ALG_FREAD_BUFFER_SIZE;

    inline char getc_fread() {
        if (sn_alg_fread_s >= sn_alg_fread_buf + SN_ALG_FREAD_BUFFER_SIZE) {
            sn_alg_fread_sz = fread_adj(sn_alg_fread_buf, sizeof(char), SN_ALG_FREAD_BUFFER_SIZE, stdin);
            sn_alg_fread_s = sn_alg_fread_buf;
        }
        return *(sn_alg_fread_s++);
    }

    inline void read_szt(size_t& x) {
        //or use fread(input, 1 << 31, stdin)

        /*register*/ char c = getchar_adj(); x = 0;
        while (!isdigit(c)) {
            c = getchar_adj();
        }
        while (isdigit(c)) {
            x = (x << 3) + (x << 1) + c - 48;
            c = getchar_adj();
        }
    }
    
    inline void read_int(int& x) {
        //or use fread(input, 1 << 31, stdin)
        /*register*/ char c = getchar_adj(); x = 0; short f = 1;
        while (!isdigit(c)) {
            if (c == '-') f = -1;
            c = getchar_adj();
        }
        while (isdigit(c)) {
            x = (x << 3) + (x << 1) + c - 48;
            c = getchar_adj();
        }
        x *= f;
    }

    inline bool read_int_eof(int& x) {
        //or use fread(input, 1 << 31, stdin)
        /*register*/ char c = getchar_adj(); x = 0; short f = 1;
        if (c == EOF) return false;
        while (!isdigit(c)) {
            if (c == '-') f = -1;
            c = getchar_adj();
            if (c == EOF) return false;        
        }
        while (isdigit(c)) {
            x = (x << 3) + (x << 1) + c - 48;
            c = getchar_adj();
        }
        x *= f;
        return true;
    }
    
    inline void read_szt_f(size_t& x) {
        //or use fread(input, 1 << 31, stdin)

        /*register*/ char c = getc_fread(); x = 0;
        while (!isdigit(c)) {
            c = getc_fread();
        }
        while (isdigit(c)) {
            x = (x << 3) + (x << 1) + c - 48;
            c = getc_fread();
        }
    }
    
    inline void read_int_f(int& x) {
        //or use fread(input, 1 << 31, stdin)
        /*register*/ char c = getc_fread(); x = 0; short f = 1;
        while (!isdigit(c)) {
            if (c == '-') f = -1;
            c = getc_fread();
        }
        while (isdigit(c)) {
            x = (x << 3) + (x << 1) + c - 48;
            c = getc_fread();
        }
        x *= f;
    }

    inline bool read_int_eof_f(int& x) {
        //or use fread(input, 1 << 31, stdin)
        /*register*/ char c = getc_fread(); x = 0; short f = 1;
        if (sn_alg_fread_s > sn_alg_fread_buf + sn_alg_fread_sz) return false;
        while (!isdigit(c)) {
            if (c == '-') f = -1;
            c = getc_fread();
            if (sn_alg_fread_s > sn_alg_fread_buf + sn_alg_fread_sz) return false;        
        }
        while (isdigit(c)) {
            x = (x << 3) + (x << 1) + c - 48;
            c = getc_fread();
        }
        x *= f;
        return true;
    }

    
    inline void write_szt(size_t x) {
        //or use fwrite(output, 1, strlen(output), stdout)
        int cnt = 0;
        char c[15];
        while (x) {
            ++cnt;
            c[cnt] = static_cast<char>((x % 10) + 48);
            x /= 10;
        }
        while (cnt) {
            putchar_adj(c[cnt]);
            --cnt;
        }
    }

    inline void write_int(int x) {
        //or use fwrite(output, 1, strlen(output), stdout)
        int cnt = 0;
        char c[16];
        while (x) {
            ++cnt;
            c[cnt] = static_cast<char>((x % 10) + 48);
            x /= 10;
        }
        if (x < 0) {
            c[15] = '-';
            ++cnt;
        }			
        while (cnt) {
            putchar_adj(c[cnt]);
            --cnt;
        }
    }

    // https://linux.die.net/man/3/ffsll
    inline unsigned int log2(unsigned int x) {
        unsigned int ret;
        __asm__ __volatile__(
            "bsr %1, %%eax"
            :"=a"(ret)
            :"m"(x)
        );
        return ret;
    }

    // calculate a * b mod m
    inline int mul_mod(int a, int b, int m) {
        int ret;
        __asm__ __volatile__(
            "tmull %%ebx\n" \
            "tdivl %%ecx\n"
            :"=d"(ret)
            :"a"(a)
            ,"b"(b), "c"(m)
        );
        return ret;
    }

    inline long long mul_mod_ll(long long a, long long b, long long m) {
        long long d = static_cast<long long>(floor(a * static_cast<double>(b) / m + 0.5));
        long long ret = a * b - d * m; // UB
        if (ret < 0) ret += m;
        return ret;
    }


    auto rmq_st(std::vector<int>& arr, size_t n) {
        size_t log2n = log2(n) + 1;
        std::vector<std::vector<int>> m(n, std::vector<int>(log2n, 0));
        for (size_t i = 0; i < n; ++i) {
            m[i][0] = i;
        }
        for (size_t j = 1; j < log2n; ++j) {
            for (size_t i = 0; i + (1 << j) - 1 < n; ++i) {
                if (arr[m[i][j - 1]] < arr[m[i + (1 << (j - 1))][j - 1]]) {
                    m[i][j] = m[i][j - 1];
                } else {
                    m[i][j] = m[i + (1 << (j - 1))][j - 1];
                }
            }
        }
        auto rmq_query = [=](size_t i, size_t j) {
            size_t k = log2(j - i + 1);
            if (arr[m[i][k]] < arr[m[j - (1 << k) + 1][k]]) {
                return m[i][k];
            } else {
                return m[j - (1 << k) + 1][k];
            }
        };
        return rmq_query;
    }

}


#endif