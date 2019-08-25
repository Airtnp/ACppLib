#ifndef SN_ALGDS_BIN_H
#define SN_ALGDS_BIN_H

#include <bits/stdc++.h>
using namespace std;

namespace binary {
    
    //For some other algs, ref: http://graphics.stanford.edu/~seander/bithacks.html
    //For linux bitops, ref: https://code.woboq.org/linux/linux/arch/x86/include/asm/bitops.h.html
    inline int lowbit(const unsigned long long& x) {
        return x & (-x);  // or ((x-1) ^ x) & x) or 1 << __builtin_ctz(x);
    }

    inline int remove_lowbit(const unsigned long long& x) {
        return x & (x - 1);  // or x ^= lowbit(x)
    }

    // __builtin_clz
    // bsr
    inline int count_leading_zeros(unsigned long long x) {
        int count;
        while (x) {
            count++;
            x ^= lowbit(x);
        }
        return count;
    }

    // __builtin_ctz
    // bsl
    inline int count_trailing_zeros(unsigned long long x) {
        int count;
        unsigned long long l = lowbit(x);
        while (x) {
            count++;
            x /= 2;
        }
        return count;
    }

    /*
        static char ntz_table[] = {
            32,0,1,12,2,6,00,13,  
            3,00,7,00,00,00,00,14,
            10,4,00,00,8,00,00,25,
            00,00,00,00,00,21,27,15,
            31,11,5,00,00,00,00,00, 
            9,00,00,24,00,00,20,26,
            30,00,00,00,23,00,19, 
            29,00,22,18,28,17,16,00
        };
        // de Bruijn sequence
        // @ref: https://www.ptt.cc/bbs/C_and_CPP/M.1246890591.A.459.html
        // @ref: http://supertech.csail.mit.edu/papers/debruijn.pdf
        inline int ntz(uint32_t x) {
            x = lowbit(x) * 0x450fbaf; 
            // any number with consecutive 6 number different
            // 100010100001111101110101111b
            return ntz_table[(x >> 26) & 0x3F];
        }
    */

    // For detail, see http://www.matrix67.com/blog/archives/264 
    // divide and conquer 11001100 -> 11001100 -> 00110011 -> 00001100
    // For __builtin_popcount, get popcount table, see http://www.xuebuyuan.com/828691.html
    inline unsigned long long count_bits(unsigned long long x) {
        x = (x & 0x5555555555555555LL) + ((x & 0xAAAAAAAAAAAAAAAALL) >> 1);
        x = (x & 0x3333333333333333LL) + ((x & 0xCCCCCCCCCCCCCCCCLL) >> 2);
        x = (x & 0x0F0F0F0F0F0F0F0FLL) + ((x & 0xF0F0F0F0F0F0F0F0LL) >> 4);
        x = (x & 0x00FF00FF00FF00FFLL) + ((x & 0xFF00FF00FF00FF00LL) >> 8);
        x = (x & 0x0000FFFF0000FFFFLL) + ((x & 0xFFFF0000FFFF0000LL) >> 16);
        x = (x & 0x00000000FFFFFFFFLL) + ((x & 0xFFFFFFFF00000000LL) >> 32);
        return x;
    }

    // divide and conquer 11001100 -> 11001100 -> 00110011 -> 00110011
    // For table-lookup, see http://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
    inline long long reverse_bits(unsigned long long x) {
        x = ( (x >> 1)  & 0x5555555555555555LL) + ( (x << 1)  & 0xAAAAAAAAAAAAAAAALL);
        x = ( (x >> 2)  & 0x3333333333333333LL) + ( (x << 2)  & 0xCCCCCCCCCCCCCCCCLL);
        x = ( (x >> 4)  & 0x0F0F0F0F0F0F0F0FLL) + ( (x << 4)  & 0xF0F0F0F0F0F0F0F0LL);
        x = ( (x >> 8)  & 0x00FF00FF00FF00FFLL) + ( (x << 8)  & 0xFF00FF00FF00FF00LL);
        x = ( (x >> 16) & 0x0000FFFF0000FFFFLL) + ( (x << 16) & 0xFFFF0000FFFF0000LL);
        x = ( (x >> 32) & 0x00000000FFFFFFFFLL) + ( (x << 32) & 0xFFFFFFFF00000000LL);
        return x;
    }

    // __builtin_parity get number of 1 odd/even

    inline long long binary_exp(long long x, long long y) {
        if (y <= 0) return 1;
        if (y % 2 == 0)
            return binary_exp(x * x, y / 2);
        else
            return x * binary_exp(x, y - 1);
    }

    inline bool get_k_bit(long long x, long long k) {
        return (x >> k) & 1;
    }

    inline bool set_k_bit(long long x, bool k_set) {
        if (k_set == 0)
            return (x &= ~(1 << k));
        else
            return (x |= 1 << k);
    }

    inline bool and_all(long long x) {
        return !(x & (x - 1)) && x;
    }

    inline bool adj_true(long long x) {
        return (x >> 1) & x;
    }

    inline bool adj_true_3(long long x) {
        return (x >> 2) & (x >> 1) & x;
    }

    inline int bin_average(int x, int y) {
        return (x & y) + ((x ^ y) >> 1);
    }

    float rsqrt(float x) {
        long i = *reinterpret_cast<long*>(&x);
        i = 0x5f3759df - (i >> 1);
        return *reinterpret_cast<float*>(&x);
    }

    // @ref: http://www.bullshitmath.lol/FastRoot.slides.html
    // Use i2f(M + C * f2i(x)) to approximate x^C with constraint that qpow(1) == 1
    // Refinement for -1/2 @ref: http://www.phailed.me/2014/10/0x5f400000-understanding-fast-inverse-sqrt-the-easyish-way/#Epilogue_%E2%80%93_The_Hard_Parts
    float qpow(float x, float exponent) {
        constexpr uint32_t MAGIC = 0x3f800000;
        long i = *reinterpret_cast<long*>(&x);
        i = (1 - exponent) * MAGIC + exponent * i;
        return *reinterpret_cast<float*>(&x);
    }

    // @ref: http://www.phailed.me/2012/08/somewhat-fast/
    inline float approx_nthroot(float x, int n) {
        constexpr uint32_t MAN_MASK = 0x7fffff;
        constexpr uint32_t EXP_MASK = 0x7f800000;
        constexpr uint32_t EXP_BIAS = 0x3f800000;
        uint32_t bits = *reinterpret_cast<int*>(&x); // obviously ub
        uint32_t man = bits & MAN_MASK; // mantissa
        uint32_t expo = (bits & EXP_MASK) - EXP_BIAS; // exponents
        uint32_t res = (man + man / n) | ((EXP_BIAS + expo / n) & EXP_MASK);
        return *reinterpret_cast<float*>(&res)
    }

    // some other float hacks @ref: https://github.com/leegao/float-hacks/blob/master/README.md
}



#endif