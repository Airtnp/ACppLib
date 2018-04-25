#ifndef SN_STRING_STRING_VIEW_H
#define SN_STRING_STRING_VIEW_H

#include "../sn_CommonHeader.h"

namespace sn_String {
    template <typename T>
    struct less_than {
        bool operator<=(const T& rhs) {
            return static_cast<T*>(this)->operator<(rhs) || static_cast<T*>(this)->operator==(rhs);
        }
        bool operator>(const T& rhs) {
            return !static_cast<T*>(this)->operator<(rhs);
        }
        bool operator>=(const T& rhs) {
            return !static_cast<T*>(this)->operator<(rhs) || static_cast<T*>(this)->operator==(rhs);
        }
        bool operator!=(const T& rhs) {
            return !static_cast<T*>(this)->operator==(rhs);
        }
    };

    // God, I really wish I can use C++17
    template <typename CharT, typename Traits = std::char_traits<CharT>>
    class basic_string_view : public less_than<basic_string_view<CharT, Traits>> {
    public:
        using value_type = CharT;
        using pointer = CharT*;
        using reference = CharT&;
        using iterator = CharT*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_pointer = const CharT*;
        using const_reference = const CharT&;
        using const_iterator = const_pointer;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        static constexpr const size_type npos = -1;
    private:
        value_type* m_data;
        size_type m_size;
    public:
        basic_string_view() noexcept : m_data{nullptr}, m_size{0} {}
        basic_string_view(const value_type* data, size_type sz)
            : m_data{const_cast<value_type*>(data)}, m_size{sz} {
            if (sz != 0 && data == nullptr) {
                throw std::runtime_error("Pass nullptr to string_view.");
            }
        }
        basic_string_view(const value_type* data) 
            : m_data{const_cast<value_type*>(data)}, m_size{Traits::length(data)} {}

        template <typename Allocator>
        basic_string_view(const std::basic_string<CharT, Traits, Allocator>& str) noexcept 
            : m_data{const_cast<value_type*>(str.data())}, m_size{str.size()} {}

        basic_string_view(const basic_string_view&) = default;
        basic_string_view& operator=(const basic_string_view&) = default;
        // non-standard ctors
        basic_string_view(basic_string_view&&) = default;
        basic_string_view& operator=(basic_string_view&&) = default;

        pointer data() noexcept { return m_data; }

        // non-standard, modified to const version
        iterator begin() noexcept { return m_data; }
        iterator end() noexcept { return m_data + m_size; }
        const_iterator cbegin() noexcept { return m_data; }
        const_iterator cend() noexcept { return m_data + m_size; }
        
        reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }
        reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }
        reverse_iterator crbegin() noexcept { return reverse_iterator{end()}; }
        reverse_iterator crend() noexcept { return reverse_iterator{begin()}; }
        

        const_iterator begin() const noexcept { return m_data; }
        const_iterator end() const noexcept { return m_data + m_size; }
        const_iterator cbegin() const noexcept { return m_data; }
        const_iterator cend() const noexcept { return m_data + m_size; }
        
        const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{end()}; }
        const_reverse_iterator rend() const noexcept { return const_reverse_iterator{begin()}; }
        const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{end()}; }
        const_reverse_iterator crend() const noexcept { return const_reverse_iterator{begin()}; }
        
        size_type size() const noexcept { return m_size; }
        size_type length() const noexcept { return m_size; }
        bool empty() const noexcept { return m_size == 0; }

        reference operator[](size_type pos) {
            return m_data[pos];
        }
        reference at(size_type pos) {
            if (pos >= m_size) {
                throw std::out_of_range("basic::string_view::at out of range");
            }
            return m_data[pos];
        }
        reference front() noexcept {
            return m_data[0];
        }
        reference last() noexcept {
            return m_data[m_size - 1];
        }

        const_reference operator[](size_type pos) const {
            return m_data[pos];
        }
        const_reference at(size_type pos) const {
            if (pos >= m_size) {
                throw std::out_of_range("basic::string_view::at out of range");
            }
            return m_data[pos];
        }
        const_reference front() const noexcept {
            return m_data[0];
        }
        const_reference last() const noexcept {
            return m_data[m_size - 1];
        }

        void clear() noexcept {
            m_data = nullptr;
            m_size = 0;
        }

        void remove_prefix(size_type n) noexcept {
            if (n > m_size) {
                n = m_size;
            }
            m_data += n;
            m_size -= n;
        }
        void remove_suffix(size_type n) noexcept {
            if (n > m_size) {
                n = m_size;
            }
            m_size -= n;
        }

        // non-standard
        void remove_prefixc(CharT c) noexcept {
            while (*m_data == c && m_size != 0) {
                ++m_data;
                --m_size;
            }
        }
        void remove_suffixc(CharT c) noexcept {
            while (*(m_data + m_size - 1) == c && m_size != 0) {
                --m_size;
            }
        }

        void swap(basic_string_view&& rhs) noexcept {
            std::swap(m_data, rhs.m_data);
            std::swap(m_size, rhs.m_size);
        }

        template <typename Allocator>
        explicit operator std::basic_string<CharT, Traits, Allocator>() const {
            return std::basic_string<CharT, Traits, Allocator>{begin(), end()};
        }

        size_type copy(CharT* s, size_type n, size_type pos = 0) const {
            if (pos > m_size) {
                throw std::out_of_range("basic_string_view::copy out of range");
            }
            std::copy_n(m_data + pos, std::min(n, m_size - pos), s);
            return std::min(n, m_size - pos);
        }
        basic_string_view substr(size_type pos = 0, size_type n = npos) const {
            if (pos > m_size) {
                throw std::out_of_range("basic_string_view::substr out of range");
            }
            return basic_string_view(m_data + pos, std::min(n, m_size - pos));
        }
        int compare(basic_string_view rhs) const noexcept {
            int res = Traits::compare(m_data, rhs.m_data, std::min(m_size, rhs.m_size));
            if (res == 0) {
                if (m_size == rhs.m_size) {
                    res = 0;
                } else {
                    res = m_size > rhs.m_size;
                }
            }
            return res;
        }
        size_type find(CharT c, size_type pos = 0) const noexcept {
            if (pos > m_size) {
                return npos;
            }
            const_iterator it = std::find(
                begin() + pos, end(), c
            );
            if (it == end()) {
                return npos;
            }
            return std::distance(begin(), it);
        }
    /*
        template <typename Pr>
        size_type find(Pr pred, size_type pos = 0) const noexcept {
            if (pos > m_size) {
                return npos;
            }
            const_iterator it = std::find_if(
                begin() + pos, end(), pred
            );
            if (it == end()) {
                return npos;
            }
            return std::distance(begin(), it);
        }
    */
        size_type find(basic_string_view sv, size_type pos = 0) const noexcept {
            if (pos > m_size) {
                return npos;
            }
            // Better use boyer-moore search
            const_iterator it = std::search(
                begin() + pos, end(), sv.begin(), sv.end()
            );
            if (it == end()) {
                return npos;
            }
            return std::distance(begin(), it);
        }
        size_type rfind(CharT c, size_type pos = 0) const noexcept {
            if (pos > m_size) {
                return npos;
            }
            const_iterator it = std::find(
                rbegin() + pos, rend(), c
            );
            if (it == rend()) {
                return npos;
            }
            return std::distance(rbegin(), it);
        }
        size_type rfind(basic_string_view sv, size_type pos = 0) const noexcept {
            if (pos > m_size) {
                return npos;
            }
            const_iterator it = std::search(
                rbegin() + pos, rend, sv.rbegin(), sv.rend()
            );
            if (it == rend()) {
                return npos;
            }
            return std::distance(rbegin(), it);
        }
    /*
        template <typename Pr>
        size_type rfind(Pr pred, size_type pos = 0) const noexcept {
            if (pos > m_size) {
                return npos;
            }
            const_iterator it = std::find_if(
                rbegin() + pos, rend(), pred
            );
            if (it == rend()) {
                return npos;
            }
            return std::distance(begin(), it);
        }
    */
        // find_xxx_of the same.
        
        // non-standard, and should be a non-member function
        // Rely on NRVO optimization
        std::vector<basic_string_view> split(CharT delim, bool keep_empty = false) const noexcept {
            std::vector<basic_string_view> v;
            size_type last_beg = 0;
            size_type last_end = find(delim);
            for (; last_end != npos; 
                last_beg = last_end + 1, last_end = find(delim, last_beg)) {
                if (last_beg != last_end || keep_empty) {
                    v.push_back(basic_string_view{m_data + last_beg, last_end - last_beg});
                }
            }
            if (last_beg != last_end || keep_empty) {
                v.push_back(basic_string_view{m_data + last_beg, m_size - last_beg});
            }
            return v;
        }
        template <typename Pr>
        std::vector<basic_string_view> split(Pr pred, bool keep_empty = false) const noexcept {
            std::vector<basic_string_view> v;
            size_type last_beg = 0;
            size_type last_end = find(pred);
            for (; last_end != npos; 
                last_beg = last_end + 1, last_end = find(pred, last_beg)) {
                if (last_beg != last_end || keep_empty) {
                    v.push_back(basic_string_view{m_data + last_beg, last_end - last_beg});
                }
            }
            if (last_beg != last_end || keep_empty) {
                v.push_back(basic_string_view{m_data + last_beg, m_size - last_beg});
            }
            return v;
        }
        void split(CharT delim, std::vector<basic_string_view>& v, bool keep_empty = false) const noexcept {
            size_type last_beg = 0;
            size_type last_end = find(delim);
            for (; last_end != npos; 
                last_beg = last_end + 1, last_end = find(delim, last_beg)) {
                if (last_beg != last_end || keep_empty) {
                    v.push_back(basic_string_view{m_data + last_beg, last_end - last_beg});
                }
            }
            if (last_beg != m_size || keep_empty) {
                v.push_back(basic_string_view{m_data + last_beg, m_size - last_beg});
            }
        }

        enum SV_REPL_ERR {
            REPL_LEN_OVERFLOW,
            REPL_INVALID,
            REPL_POS_OUTRANGE,
            REPL_TRUNCATED
        };

        constexpr static const size_t repl_max = 64;

    /*
        // Better optional
        std::pair<bool, long long> to_integer(size_type pos = 0, int base = 10) {
            if (m_size > repl_max) {
                return std::make_pair(false, REPL_LEN_OVERFLOW);
            }
            if (pos > m_size) {
                return std::make_pair(false, REPL_POS_OUTRANGE);
            }
            errno = 0;
            CharT val[repl_max + 1];
            copy(val, m_size, pos);
            val[m_size - pos] = '\0';
            CharT* end;
            long long res = strtoll(val, &end, base);
            if (res == 0 && errno != 0) {
                return std::make_pair(false, REPL_INVALID);
            } else if (errno == 0 && *end != '\0') {
                return std::make_pair(false, REPL_TRUNCATED);
            } else if (errno != 0) {
                return std::make_pair(false, errno);
            }
            return std::make_pair(true, res);
        }

        std::pair<bool, long double> to_double(size_type pos = 0, int base = 10) {
            if (m_size > repl_max) {
                return std::make_pair(false, REPL_LEN_OVERFLOW);
            }
            if (pos > m_size) {
                return std::make_pair(false, REPL_POS_OUTRANGE);
            }
            errno = 0;
            CharT val[repl_max + 1];
            copy(val, m_size, pos);
            val[m_size - pos] = '\0';
            CharT* end;
            long long res = strtold(val, &end, base);
            if (res == 0 && errno != 0) {
                return std::make_pair(false, REPL_INVALID);
            } else if (errno == 0 && *end != '\0') {
                return std::make_pair(false, REPL_TRUNCATED);
            } else if (errno != 0) {
                return std::make_pair(false, errno);
            }
            return std::make_pair(true, res);
        }

        // non-standard
        void append(basic_string_view rhs) noexcept {
            // assert(m_data + m_size == rhs.m_data)
            m_size += rhs.m_size;
        }

        basic_string_view operator+(basic_string_view rhs) noexcept {
            // assert(m_data + m_size == rhs.m_data)
            return basic_string_view{m_data, m_size + rhs.m_size};
        }
        basic_string_view& operator+=(basic_string_view rhs) noexcept {
            // assert(m_data + m_size == rhs.m_data)
            m_size += rhs.m_size;
            return *this;
        }
    */

        bool operator==(const basic_string_view rhs) const noexcept {
            if (m_size != rhs.m_size) {
                return false;
            }
            return compare(rhs) == 0;
        }
        bool operator<(const basic_string_view rhs) const noexcept {
            return compare(rhs) < 0;
        }
        template <typename XCharT, typename XTraits>
        friend std::basic_ostream<XCharT, XTraits>& operator<<(std::basic_ostream<XCharT, XTraits>& os, basic_string_view sv) {
            os.write(sv.m_data, sv.m_size);
            return os;
        }
    };

    using string_view = basic_string_view<char>;

    string_view operator""_sv (const char* str, size_t len) noexcept {
        return string_view{str, len};
    }
}

// UB
namespace std {
    template <typename CharT, typename Traits>
    struct hash<sn_String::basic_string_view<CharT, Traits>> : public unary_function<sn_String::basic_string_view<CharT, Traits>, size_t> {
        size_t operator()(const sn_String::basic_string_view<CharT, Traits>& sv) const noexcept {
            return hash<std::basic_string<CharT, Traits, std::allocator<CharT>>>(sv);
        }
    };
}
