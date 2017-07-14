// ref: https://medium.com/@barryrevzin/the-vector-monad-in-c-really-without-the-ugly-stuff-3112137db5d7
#include <vector>
#include <iostream>
#include <functional>

template <class F>
struct map_t {
    F f;
    
    template <class T, class U = std::result_of_t<F&(T const&)>>
    friend std::vector<U> operator|(std::vector<T> const& v, map_t m) {
        std::vector<U> res;
        for (T const& elem : v) {
            res.push_back(m.f(elem));
        }
        return res;
    }
};

template <class F> map_t<std::decay_t<F>> map(F&& f) { return {std::forward<F>(f)}; }

template <class F>
struct and_then_t {
    F f;
    
    template <class T, class U = std::result_of_t<F&(T const&)>>
    friend U operator|(std::vector<T> const& v, and_then_t a) {
        U result;
        for (T const& elem : v) {
            U next = a.f(elem);
            result.insert(result.end(), next.begin(), next.end());
        }
        return result;
    }
};

template <class F> and_then_t<std::decay_t<F>> and_then(F&& f) { return {std::forward<F>(f)}; }

int f1(int a) { return a * a; }
std::vector<int> f2(int b, int c) {
    std::vector<int> v;
    for (int i =b; i <= c; ++i) {
        v.push_back(i);
    }
    return v;
}
int f3(int d) { return 2 * d; }


int main() {
    std::vector<int> results = std::vector{1,2,3} | and_then([](int a){
        return std::vector{3,4,5} | and_then([=](int b){
            return f2(f1(a),f1(b)) | map(f3);
        });});
    
    for (int i : results) { 
        std::cout << i << ' ';
    }
}