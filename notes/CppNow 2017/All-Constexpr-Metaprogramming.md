# All about constexpr metaprogramming

## constexpr ALL the things

[video](https://www.youtube.com/watch?v=HMB9oXFobJc&index=9&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)

* implement parserc in constexpr C++ template
* constexpr string
```c++
struct static_string {
    template <std::size_t N>
    constexpr static_string(const char (&str)[N])
        : m_size(N-1), m_data(&str[0]) {}
};
```
* constexpr vector (fixed size) (just put array) (can push_back)
* constexpr mutable string
* + vector of chars
* constexpr map
* + `std::array<cx::pair<Key, Value>, Size>`
* + `no constexpr operator=` in `std::pair`
* no `std::variant`
* + missing some constexpr support (ctor)
* + but it can! (see versatile)
* constexpr allocator
* parser build up (see sn_RegexT)
* + return everything lambda (constexpr in Cpp17)
* + change result type of a parser (fmap)
* + run one parser, second based what first return (bind)
* + run one, if it fails run another (operator|/<|>)
* + run two in succession and combine the outputs (combine/<*>)
```c++
// combine :: Parser a -> Parser b -> (a - > b -> c) -> Parser c
template <typename P1, typename P2, typename F, typename R = std::result_of_t<F(parse_t<P1>, parse_t<P2>)>>
constexpr auto combine(P1&& p1, P2&& p2, F&& f) {
    return [=](parse_input_t i) = > parse_result_t<R> {
        constexpr const auto r1 = p1(i);
        if (!r1) return std::nullopt;
        constexpr const auto r2 = p2(i);
        if (!r2) return std::nullopt;
        return parse_result_t<R>{
            cx::make_pair(f(r1->first, r2->first), r2->second);
        };
    }
}
```
* constexpr dtor problem 
* debugging problem
* constexpr STL
* + algorithm except stable_sort/inplace_merge/stable_partition (complexity problem)
* + iterator (can be constexpr by constexpr container)