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
            std::size_t m_size = 0;
            // T m_data[N] {};
            std::aligned_storage_t<sizeof(T), alignof(T)> m_data[N];
        public:
            using iterator = T*;
            
            static_vector() {}
            constexpr std::size_t size() const { return m_size; }
            constexpr T& operator[](std::size_t n)
            { return *reinterpret_cast<T*>(m_data + n); }
            constexpr const T& operator[](std::size_t n) const
            { return *reinterpret_cast<const T*>(m_data + n); }
            constexpr iterator begin() { return m_data; }
            constexpr iterator end() { return (m_data + N); }
            template <typename ...Args>
            void emplace_back(Args&&... args) {
                if (m_size >= N)
                    throw std::bad_alloc{};
                new (m_data + m_size) T{std::forward<Args>(args)...};
                ++m_size;
            }
            ~static_vector() {
                for (std::size_t pos = 0; pos < m_size; ++pos) {
                    reinterpret_cast<T*>(m_data + pos)->~T();
                }
            }
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

        template <typename T, std::size_t ...Dimensions>
        struct static_multi_array {
            constexpr const T& M_access() const { return M_data; }
            T M_data;
        };

        template <typename T, std::size_t first, std::size_t ...rest>
        struct static_multi_array<T, first, rest...> {
            template <typename ...Args>
            constexpr const T& M_access(std::size_t first_idx, Args&&... rest_idxs) const {
                return M_arr[first_idx].M_access(rest_idxs...);
            }
            static_multi_array<T, rest...> M_arr[first];
        };

        template <typename T, std::size_t ...Dimensions>
        struct static_multi_access_array;

        template <typename T, std::size_t first, std::size_t ...rest>
        struct static_multi_access_array<T, first, rest...> {
            constexpr const T& operator[](std::size_t idx) const {
                return M_arr[idx];
            }
            static_multi_access_array<T, rest...> M_arr[first];
        };

        template <typename T, std::size_t first>
        struct static_multi_access_array<T, first> {
            constexpr const T& operator[](std::size_t idx) const { 
                return M_arr[idx]; 
            }
            T M_arr[first];
        };
    }

}