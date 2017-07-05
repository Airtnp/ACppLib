// ref: https://www.zhihu.com/question/62012225
#include <tuple>
#include <type_traits>

template <typename T, typename... Any>
decltype( void( T{std::declval<Any>()...} ), std::true_type{} )
constructible_test( int );

template <typename, typename...> std::false_type
constructible_test( ... );

template <typename T, typename... Any>
using is_constructible = decltype( constructible_test<T, Any...>( 0 ) );

struct any_type
{
    template<typename T>
    constexpr operator T() const;
};

template<typename T>
auto tuple_binding( T&& object ) noexcept
{
    using type = std::decay_t<T>;
    //3
    if constexpr( is_constructible<type, any_type, any_type, any_type>{} )
    {
        auto&& [p1, p2, p3] = std::forward<T&&>(object);
        return std::make_tuple( p1, p2, p3 );
    }
    //2
    else if constexpr( is_constructible<type, any_type, any_type>{} )
    {
        auto&& [p1, p2] = std::forward<T&&>(object);
        return std::make_tuple( p1, p2 );
    }
    //1
    else if constexpr( is_constructible<type, any_type>{} )
    {
        auto&& [p1] = std::forward<T&&>(object);
        return std::make_tuple( p1 );
    }
    else
        return std::make_tuple();
}