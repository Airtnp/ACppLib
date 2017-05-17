#ifndef SN_META_H
#define SN_META_H

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

    namespace constexpr_container {
        template <typename T, std::size_t N>
        class array_result {
        private:
            constexpr static std::size_t m_size = N;
            T m_data[N] {};
        public:
            constexpr std::size_t size() const { return N; }
            constexpr T& operator[](std::size_t n)
            { return m_data[n]; }
            constexpr const T& operator[](std::size_t n) const
            { return m_data[n]; }
            using iterator = T*;
            constexpr iterator begin() { return &m_data[0]; }
            constexpr iterator end() { return &m_data[N]; }
        };
    }

    namespace constexpr_math {
        // complete 14-support
        constexpr double pow_int(double base, int exp) {
            if (exp < 0) {
                base = 1.0 / base;
                exp = -1 * exp;
            }
            double res = 1.0;
            for (std::size_t i = 0; i < exp; ++i)
                res *= base;
            return res;
        }

        template <typename T = std::uint32_t>
        constexpr T bin(const char* t) {
            T x = 0;
            std::size_t b = 0;
            for (std::size_t i = 0; t[i] != '\0'; ++i) {
                if (b >= std::numeric_limits<T>::digits)
                    throw std::overflow_error("Overflow bits");
                switch(t[i]) {
                    case ',': break;
                    case '0': x *= 2; ++b; break;
                    case '1': x = (2*x)+1; ++b; break;
                    default:
                        throw std::domain_error("Only 0, 1, ',' are allowed");
                }
            }
            return x;
        }
    }

   

}











#endif