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

    //__builtin_clz
    inline int count_leading_zeros(unsigned long long x) {
        int count;
        while (x) {
            count++;
            x ^= lowbit(x);
        }
        return count;
    }

    // __builtin_ctz
    inline int count_trailing_zeros(unsigned long long x) {
        int count;
        unsigned long long l = lowbit(x);
        while (x) {
            count++;
            x /= 2;
        }
        return count;
    }

    //For detail, see http://www.matrix67.com/blog/archives/264 
    //divide and conquer 11001100 -> 11001100 -> 00110011 -> 00001100
    //For __builtin_popcount, get popcount table, see http://www.xuebuyuan.com/828691.html
    inline unsigned long long count_bits(unsigned long long x) {
        x = (x & 0x5555555555555555LL) + ((x & 0xAAAAAAAAAAAAAAAALL) >> 1);
        x = (x & 0x3333333333333333LL) + ((x & 0xCCCCCCCCCCCCCCCCLL) >> 2);
        x = (x & 0x0F0F0F0F0F0F0F0FLL) + ((x & 0xF0F0F0F0F0F0F0F0LL) >> 4);
        x = (x & 0x00FF00FF00FF00FFLL) + ((x & 0xFF00FF00FF00FF00LL) >> 8);
        x = (x & 0x0000FFFF0000FFFFLL) + ((x & 0xFFFF0000FFFF0000LL) >> 16);
        x = (x & 0x00000000FFFFFFFFLL) + ((x & 0xFFFFFFFF00000000LL) >> 32);
        return x;
    }

    //divide cand conquer 11001100 -> 11001100 -> 00110011 -> 00110011
    //For table-lookup, see http://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
    inline long long reverse_bits(unsigned long long x) {
        x = ( (x >> 1)  & 0x5555555555555555LL) + ( (x << 1)  & 0xAAAAAAAAAAAAAAAALL);
        x = ( (x >> 2)  & 0x3333333333333333LL) + ( (x << 2)  & 0xCCCCCCCCCCCCCCCCLL);
        x = ( (x >> 4)  & 0x0F0F0F0F0F0F0F0FLL) + ( (x << 4)  & 0xF0F0F0F0F0F0F0F0LL);
        x = ( (x >> 8)  & 0x00FF00FF00FF00FFLL) + ( (x << 8)  & 0xFF00FF00FF00FF00LL);
        x = ( (x >> 16) & 0x0000FFFF0000FFFFLL) + ( (x << 16) & 0xFFFF0000FFFF0000LL);
        x = ( (x >> 32) & 0x00000000FFFFFFFFLL) + ( (x << 32) & 0xFFFFFFFF00000000LL);
        return x;
    }

    inline long long binary_exp(long long x, long long y) {
        if (y <= 0) return 1;
        if (y % 2 == 0)
            return binary_exp(x * x, y / 2);
        else
            return x * binary_exp(x, y - 1);
    }

}



#endif