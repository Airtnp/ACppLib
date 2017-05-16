#ifndef SN_META_H
#define SN_META_H

namespace sn_Meta {
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