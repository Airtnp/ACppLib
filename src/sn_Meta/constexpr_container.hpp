#include "../sn_CommonHeader.h"

namespace sn_Meta {
    
    namespace constexpr_container {

        // ref: https://github.com/ZaMaZaN4iK/constexpr_allocator/blob/master/test.cpp
        template <class T, unsigned N = 100>
        struct constexpr_allocator {
            T data[N] = {};
            unsigned used = 0;
            using value_type = T;
            using pointer = T*;

            constexpr pointer allocate(unsigned s) {
                pointer ret = data + used;
                used += s;
                return ret;
            }

            template< class U, class... Args >
            constexpr void construct( U* p, Args&&... args ) {
                *p = T(std::forward<Args>(args)...);
            }

            template< class U >
            constexpr void destroy( U* ) {}

            constexpr void deallocate(T* p, unsigned n ) {
                if (data + used - n == p)
                    used -= n;
            }

            static constexpr bool allocator_auto_cleanup = true;
        };  


        template <typename T, std::size_t N>
        class static_vector {
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

        template <typename T, typename ...Names>
        struct static_map {
        private:
            template <typename Name>
            struct element {
                using name = Name;
                explicit element(T v) : value(v) {}
                T value;
            };
        public:
            template <typename ...Args>
            static_map(Args&&... args)
                : m_elements(std::make_tuple(elements<Args>(std::forward<Args>(args)...)) {
                    static_assert(sizeof...(Names) == sizeof...(Args), "Not match.");
            }

            template <typename Name>
            decltype(auto) get() const {
                return std::get<element<Name>>(m_elements).value;
            }

            template <typename Name>
            void set(const T& v) {
                std::get<element<Name>>(m_elements).value = v;
            }

            template <typename Name>
            void set(T&& v) {
                std::get<element<Name>>(m_elements).value = std::move_if_noexcept(v);
            }
        private:
            std::tuple<element<Names>...> m_elements;
        };
    }

}