#include "../sn_CommonHeader.h"

namespace sn_Meta {
    
    template <typename T>
    struct instance_of {
        using type = T;
        constexpr instance_of(int = 0) {}
    };

    struct empty {
        constexpr empty (int = 0) {}
    };

    template <bool P>
    struct selector {
        static constexpr const bool value = P;
    };

    using true_type = selector<true> true_type;
    using false_type = selector<false> false_type;

    template <typename T, T V>
    struct static_parameter {};

    template <typename T, T V>
    struct static_value : static_parameter<T, V> {
        using type = T;
        constexpr const static T value = V;

        constexpr operator T() const {
            return V;
        }
        constexpr static_value(int = 0) {}
    };

    template <typename T, T V>
    inline constexpr T static_value_cast(static_value<T, V>) {
        return V;
    }

    // aware of padding
    template <typename T>
    class larger_than {
        T body[2];
    };

    template <typename T>
    static constexpr const std::size_t larger_size = sizeof(larger_than<T>);

    template <std::size_t N>
    struct fixed_char {
        using type = char[N];
    };

    template <std::size_t N>
    using fixed_size_type = fixed_char<N>::type;

    template <std::size_t N>
    static constexpr const std::size_t fixed_size = sizeof(fixed_char<N>::type);

    template <bool Statement>
    struct static_assertion {};

    // compiler-notice friendly
    template <>
    struct static_assertion<true> {
        constexpr static_assertion() {}
        template <typename T>
        constexpr static_assertion(T) {}
    };

    template <>
    struct static_assertion<false>;

    // template <bool Statement>
    // using static_assert = static_assertion<Statement>;

#define SN_META_STATIC_ASSERT(statement) \
    sizeof(sn_Meta::staic_assertion<!!(statement)>)

    // You can even inherit static_assertion<T...> to avoid instantiation

#define SN_META_STATIC_ASSERT_LEGAL(statement) \
    sizeof((statement), 0)

#define SN_META_CONST_REF_TO(statemnet) \
    (*static_cast<const T*>(0))

#define SN_META_REF_TO(statemnet) \
    (*static_cast<T*>(0))

    struct incomplete_type;
    struct complete_type {};

    template <std::size_t N>
    struct compile_time_const {
        complete_type& operator==(compile_time_const<N>) const;
        template <std::size_t K>
        in_complete_type& operator==(compile_time_const<K>) const;
    };

    template <typename T>
    compile_time_const<0> length_of(T) {
        return compile_time_const<0>{};
    }

    template <typename T, std::size_t N>
    compile_time_const<N> length_of(T(&)[N]) {
        return compile_time_const<N>{};
    }

    template <typename T, std::size_t N>
    typedef fixed_size_type<N>& not_an_array(T(&)[N]);

#define SN_META_LENGTH_OF(X) \
    sizeof(not_an_array(X))

#define SN_META_NOT_IMPLEMENTED \
    SN_META_STATIC_ASSERT(false)


#define SN_META_CONSTANT_STRUCT(...) \
    struct ANONYMOUS_VARIABLE(SN_CONSTANT_STRUCT) { \
        using value_type = decltype(__VA_ARGS__); \
        static constexpr auto value() { \
            return __VA_ARGS__; \
        } \
        constexpr operator value_type() const noexcept { \
            return value(); \
        } \
        constexpr value_type operator()() const noexcept { \
            return value(); \
        } \
    }; \


/* = or not*/
#define SN_META_CONSTANT_LAMBDA(...) \
    [] { \
        struct R { \
            using value_type = decltype(__VA_ARGS__); \
            static constexpr auto value() { \
                return __VA_ARGS__; \
            } \
            constexpr operator value_type() const noexcept { \
                return value(); \
            } \
            constexpr value_type operator()() const noexcept { \
                return value(); \
            } \
        }; \
        return R{}; \
    } \
}