#include <vector>
#include <algorithm>
#include <string>

#define RETURNS(...) noexcept(noexcept(__VA_ARGS__)) -> decltype(__VA_ARGS__){ return __VA_ARGS__; }

#define resolve_overload(f) [](auto&&... xs) RETURNS (f(::std::forward<decltype(xs)>(xs)...))

void f(int& i) {
    ++i;
}

void f(std::string& s);

int main() {
    std::vector<int> as = {1, 2, 3, 4, 5};
    // std::for_each(begin(as), end(as), f); // Error cannot deduct overload f
    std::for_each(begin(as), end(as), resolve_overload(f));    
}