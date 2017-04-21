#include <functional>
#include <iostream>

template <typename T, typename R>
struct SList
    : std::function<R(std::function<R()>, std::function<R(T, SList<T, R>)>)> {
  template <typename F>
  SList(F f)
      : std::function<R(std::function<R()>, std::function<R(T, SList<T, R>)>)>(
            f){};
};

template <typename T, typename R>
SList<T, R> Nil = SList<T, R>([](auto n, auto c) { return n(); });

template <typename T, typename R>
SList<T, R> Cons(T x, SList<T, R> xs) {
  return SList<T, R>([=](auto fx, auto fxs) { return fxs(x, xs); });
}

int main() {
  auto l = Nil<int, void>;
  for (;;) {
    int x;
    std::cin >> x;
    if (x > 0)
      l = Cons<int, void>(x, l);
    else
      break;
  }
  std::function<void()> fx = []() { std::cout << "End of list!" << std::endl; };
  std::function<void(int, SList<int, void>)> fxs = [&](auto x, auto xs) {
    std::cout << "List element " << x << std::endl;
    xs(fx, fxs);
  };
  l(fx, fxs);
  return 0;
}