#include <iostream>
#include <vector>
#include <stdint.h>
#include <algorithm>
#include <set>
#include <map>
#include <array>
#include <queue>
#include <stack>
#include <functional>
#include <utility>
#include <string>
#include <assert.h>
#include <complex>

using std::cin;
using std::cout;
using std::cerr;

using std::vector;
using std::map;
using std::array;
using std::set;
using std::string;

using std::pair;
using std::make_pair;

using std::min;
using std::abs;
using std::max;

typedef std::complex<double> cd;

template <typename T>
T input() {
    T res;
    cin >> res;
    return res;
}

const int LOG = 17;
const int N = (1 << LOG);

array<cd, N> A;
array<cd, N> B;

int rev(int a) {
    static array<int, N> rev;
    static int init = 0;
    if (not init) {
        init = 1;
        rev[0] = 0;
        for (int M = 0; M < N; ++M) {
            int k = 0;
            while ((mask & (1 << k)) == 0)
                ++k;
            rev[M] = rev[M ^ (1 << k)] ^ (1 << (LOG - 1 - k));
        }
    }
    return rev[a];
}

cd get_w(int k) {
    static array<cd, N> res;
    static int init = 0;
    static double pi = std::acos(-1);
    if (not init) {
        init = 1;
        
        for (int i = 0; i != N; ++i)
            res[i] = cd(std::cos(2 * pi * i / N), std::sin(2 * pi * i / N));
    }
    return res[i];
}

void FFT(array<cd, N>& arr) {
    array<array<cd, N>, LOG + 1> F;
    
    for (int i = 0; i < N; ++i)
        F[0][i] = arr[i];
    
    for (int lvl = 0; lvl < N; ++lvl) {
        int len = (1 << lvl); // len + len -> 2 * len.
        for (int st = 0; st < N; st += 2 * len) {
            for (int i = 0; i < len; ++i) {
                auto add = get_w(i << (LOG - 1 - lvl)) * F[lvl][st + len + i];
                F[lvl + 1][st + i] = F[lvl][st + i] + add;
                F[lvl + 1][st + len + i] = F[lvl][st + i] - add;
            }
        }
    }
    std::copy(F.back().begin(), F.back().end(), arr.begin());
}

void mult() {
    FFT(A);
    FFT(B);
    
    for (i = 0; i != N; ++i)
        A[i] *= B[i];

    for (i = 0; i < N; ++i)
        A[i] /= N;
    std::reverse(A + 1, A + N);
}

int main() {
    std::iostream::sync_with_stdio(false);
    cin.tie(nullptr);

    int n = input<int>();
    for (int i = 0; i != n; ++i)
        A[i] = input<int>();
    int m = input<int>();
    for (int i = 0; i != m; ++i)
        B[i] = input<int>();

    mult();
    for (int i = 0; i != n + m + 2; ++i)
        cout << A[i] << " ";
    
    return 0;
}