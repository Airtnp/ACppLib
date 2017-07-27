// ref: https://www.zhihu.com/question/62916223/answer/203425102

#include <type_traits>
#include <cstddef>

struct name1{};
struct name2{};
struct name3{};

template<class T, std::size_t V>
struct kv_pair {
    using key = T;
    constexpr static size_t value = V;
};

template<class... Pairs>
struct a_enum;

template<class ValuePair>
struct a_enum<ValuePair> {
    constexpr static std::integral_constant<size_t, ValuePair::value> get(typename ValuePair::key) {
        return {};
    }
};

template<class Pair, class... Pairs>
struct a_enum<Pair, Pairs...> : a_enum<Pair>, a_enum<Pairs>... {
    using a_enum<Pair>::get;
    using a_enum<Pairs>::get...;
};

template<class T, class U>
constexpr static size_t enum_get = decltype(T::get(U{}))::value;

int main() {
    using theEnum = a_enum<kv_pair<name1, 0>, kv_pair<name2, 1>, kv_pair<name3, 2>>;

    static_assert(enum_get<theEnum, name1> == 0);
    static_assert(enum_get<theEnum, name2> == 1);
    static_assert(enum_get<theEnum, name3> == 2);
}