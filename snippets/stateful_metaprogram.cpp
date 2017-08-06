// ref: https://stackoverflow.com/questions/44267673/is-stateful-metaprogramming-ill-formed-yet
// ref: https://github.com/DaemonSnake/unconstexpr

template <int N>
struct flag {
    friend constexpr int adl_flag(flag<N>);
    constexpr operator int() { return N; }
};

template <int N>
struct write {
    friend constexpr int adl_flag(flag<N>) { return N; }
    static constexpr int value = N;
};

template <int N, int = adl_flag(flag<N>{})>
constexpr int read(int, flag<N>, int R = read(0, flag<N + 1>{})) {
    return R;
}

template <int N>
constexpr int read(float, flag<N>) {
    return N;
}

template <int N = 0>
constexpr int counter(int R = write<read(0, flag<0>{}) + N>::value) {
    return R;
}

template<int = counter()>
struct S {};

#include <type_traits>

int main() {
    static_assert(counter() != counter(), "Your compiler is mad at you");
    static_assert(!std::is_same_v<S<>, S<>>, "This is ridiculous");
}

//tools
template <template<class...> class H, class... Args>
struct reader
{
    friend constexpr void adl_lookup(reader);
};

template <template<class...> class H, class... Args>
constexpr bool is_instanciated(bool R = noexcept(adl_lookup(reader<H, Args...>{}))) { return R; }

template <template<class...> class H, class... Args>
struct writer { friend constexpr void adl_lookup(reader<H, Args...>) {} };

//examples
template <class T>
struct ex : private writer<ex, T> {};

template <class T>
void require_type(int = sizeof(T)) {}

#include <iostream>

int main()
{
    std::cout << is_instanciated<ex, void>() << std::endl;
    static_assert(!is_instanciated<ex, void>());

    require_type<ex<void>>();

    std::cout << is_instanciated<ex, void>() << std::endl;
    static_assert(is_instanciated<ex, void>());
}