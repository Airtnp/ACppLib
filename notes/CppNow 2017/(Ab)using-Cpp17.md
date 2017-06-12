# (Ab)using C++17
[video](https://www.youtube.com/watch?v=AqDsso3S5fg&index=14&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)

* pointer-to-member fold `.*` and `.->`

```c++
struct S {
    void* do_things() {}
    S* lhs;
    S* rhs;
};

template <typename O, typename ...T>
decltype(auto) walk_tree(O* o, T&&... t) {
    return (o ->* ... ->* t)();
}

int main() {
    S s;
    walk_tree(&s, &S::lhs, &S::rhs, &S::rhs, &S::do_things);
}
```
* difference between (which faster)
* + `{std::is_nothrow_constructible_v<S<Is...>>, ...};` (better, less memory)
* + `{std::is_nothrow_constructible<S<Is...>>{}, ...};`
* + `std::conjuction_v<std::is_nothrow_constructible<S<Is...>>...>` (cannot compile by default ftemplate-depth)
* + `{std::is_nothrow_constructible_v<S<Is...>>{} && ...};` (better, less memory)
* + - take advantage of short-circuit!
* + if-constexpr vs SFINAE
* + - nearly same (by assembly they just same)
* destructing standard containers

```c++
template <std::size_t I, typename T, std::size_t ...Is>
auto destruture_impl(T&& t, std::index_sequence<Is...>) {
    return std::forward_as_tuple(*std::next(std::begin(std::forward<T>(t)), I+Is)...);
}

template <std::size_t I, std::size_t Cnt, typename T>
auto destructure(T&& t) {
    return destructure_impl<I>(std::forward<T>(t), std::make_index_sequence<Cnt>{});
}

template <std::size_t Cnt, typename T>
auto destructure(T&& t) {
    return destructure_impl<0>(std::forward<T>(t), std::make_index_sequence<Cnt>{});
}

int main() {
    std::vector<int> v{1, 2, 3};
    auto [a, b] = destructure<2>(v); // forward_as_tuple ensures its reference
}
```
* deduction guides work for `std::function`
```c++
template <typename Sig>
struct function_traits_impl;

template <typename R, typename ...Args>
struct function_traits_impl<R(Args...)> {
    using result_type = R;
    using function_type = R(Args...);
    constexpr static auto arity = sizeof...(Args);
    using args_type = std::tuple<Args...>;
};

template <typename Sig>
struct function_traits_impl<std::function<Sig>>
    : function_traits_impl<Sig> {};

template <typename F>
struct function_traits
    : function_traits_impl<
        decltype(std::function{std::declval<F>()})
    > {};
```
* variant: polymorphism without dynamic allocation