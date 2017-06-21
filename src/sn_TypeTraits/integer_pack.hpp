#ifndef SN_TYPETRAITS_INTEGER_PACK
#define SN_TYPETRAITS_INTEGER_PACK

#include "../sn_CommonHeader.h"

namespace sn_TypeTraits {
    template <std::size_t ...I>
    struct index_sequence {
        using type = index_sequence;
    };

    template <typename T, typename U>
    struct concate {};

    template <std::size_t ...I1, std::size_t ...I2>
    struct concate<index_sequence<I1...>, index_sequence<I2...>>
        : index_sequence<I1..., (I2 + sizeof...(I1))...> {};
    template <std::size_t N>
    struct make_index_sequence_impl
        : concate<
            typename make_index_sequence_impl<N / 2>::type,
            typename make_index_sequence_impl<N - N / 2>::type
        > {};

    template <>
    struct make_index_sequence_impl<0> : index_sequence<> {};
    template <>
    struct make_index_sequence_impl<1> : index_sequence<0> {};

    template <std::size_t N>
    using make_index_sequence = typename make_index_sequence_impl<N>::type;

    template <typename T, T ...Is>
    struct integer_pack : std::integral_constant<std::size_t, sizeof...(Is)> {
        static_assert(std::is_integral<T>::value, "integer_pack can only contain integral value.");
        using value_type = T;
        using type = integer_pack<T, Is...>;
        static constexpr const std::size_t size = sizeof...(Is); // or std::integral_constant<std::size_t, sizeof...(Is)>::value
        static constexpr const std::array<T, sizeof...(Is)> elements = {Is...};
    };

    template <std::size_t ...Is>
    using index_pack = integer_pack<std::size_t, Is...>;

    template <typename T>
    struct integer_pack_size {};

    template <typename T, T ...Is>
    struct integer_pack_size<integer_pack<T, Is...>>
        : std::integral_constant<std::size_t, sizeof...(Is)> {};

    template <typename T>
    constexpr const std::size_t integer_pack_size_v = integer_pack_size<T>::value;

    template <typename T>
    struct integer_pack_negate {};

    template <typename T, T ...Is>
    struct integer_pack_negate<integer_pack<T, Is...>>
        : integer_pack<T, (-Is)...> {};

    template <typename T>
    using integer_pack_negate_t = typename integer_pack_negate<T>::type;

    template <typename T, typename P1, typename P2>
    struct integer_pack_merge {};

    template <typename T, T ...Is1, T ...Is2>
    struct integer_pack_merge<T, integer_pack<T, Is1...>, integer_pack<T, Is2...>>
        : integer_pack<T, Is1..., Is2...> {};
    
    template <typename T, typename P1, typename P2>
    using integer_pack_merge_t = typename integer_pack_merge<T, P1, P2>::type;

    template <typename T, typename P1, typename P2>
    struct integer_pack_concate {};

    template <typename T, T ...Is1, T ...Is2>
    struct integer_pack_concate<T, integer_pack<T, Is1...>, integer_pack<T, Is2...>>
        : integer_pack<T, I1..., (I2 + sizeof...(I1))...> {};
    template <typename T, T N>
    struct make_integer_pack_impl
        : integer_pack_concate<
            typename make_integer_pack_impl<T, N / 2>::type,
            typename make_integer_pack_impl<T, N - N / 2>::type
        > {};

    template <typename T>
    struct make_integer_pack_impl<T, 0> : integer_pack<T> {};
    template <typename T>
    struct make_integer_pack_impl<T, 1> : integer_pack<T, 0> {};

    template <typename T, T N>
    using make_integer_pack = typename make_integer_pack_impl<T, N>::type;

    template <std::size_t N>
    using make_index_pack = typename make_integer_pack_impl<std::size_t, N>::type;

    template <typename T, T from, T to, T step, T nvals = (from < to ? to - from : from - to)>
    struct make_integer_pack_range_impl {
    private:
        static_assert(nvals % step == 0, "bad step value.");
        template <typename T, bool dir>
        struct make_integer_pack_range_assist {};

        template <T ...Is>
        struct make_integer_pack_range_assist<integer_pack<T, Is...>, true>
            : integer_pack<T, (from + step * Is)...> {};

        template <T ...Is>
        struct make_integer_pack_range_assist<integer_pack<T, Is...>, false>
            : integer_pack<T, (from - step * Is)...> {};
    public:
        using type = typename make_integer_pack_range_assist<
            make_integer_pack<T, 1 + nvals / step>, (from < to)
        >::type;
    };

    template <typename T, T n, T step, T nvals>
    struct make_integer_pack_range_impl<T, n, n, step, nvals>
        : integer_pack<T, n> {};

    template <typename T, T from, T to, T step>
    using make_integer_pack_range = typename make_integer_pack_range_impl<T, from, to, step>::type;

    template <std::size_t from, std::size_t to, std::size_t step>
    using make_index_pack_range = typename make_integer_pack_range_impl<std::size_t, from, to, step>::type;

    template <std::size_t I, typename T>
    struct integer_pack_at {};

    template <std::size_t I, typename T, T ...Is>
    struct integer_pack_at<I, integer_pack<T, Is...>> {
        static_assert(sizeof...(Is) != 0, "No element.");
        static_assert(I < sizeof...(Is), "Index out of bounds.");
        constexpr static const std::array<T, sizeof...(Is)> elements = {Is...};
        constexpr static const T value = elements[I];
    };

    // https://codereview.stackexchange.com/questions/133626/integer-packs-and-integer-pack-utilities-for-template-meta-programming
    // TODO: add shift/sort/max/min/union
}

#endif