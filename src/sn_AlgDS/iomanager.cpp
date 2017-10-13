#include <iostream>
#include <type_traits>
#include <cstdio>
#include <cstring>
#include <utility>
#include <iterator>
#include <initializer_list>
#include <string>

namespace IOTag {
    struct Unlocked;
    struct FRW;
    struct IOStream;
}

namespace detail {
    template <typename T>
    struct is_sint : std::false_type {};
    template <typename T>
    struct is_uint : std::false_type {};
    
    
    template <>
    struct is_sint<int> : std::true_type {};
    template <>
    struct is_sint<long> : std::true_type {};
    template <>
    struct is_sint<long long> : std::true_type {};
    template <>
    struct is_uint<unsigned int> : std::true_type {};
    template <>
    struct is_uint<unsigned long> : std::true_type {};
    template <>
    struct is_uint<unsigned long long> : std::true_type {};
    
    inline char getchar() {
#if defined(__POSIX__)
        return static_cast<char>(::getchar_unlocked());
#else
        return static_cast<char>(::getchar());
#endif
    }
    
    inline void putchar(char c) {
#if defined(__POSIX__)
        ::putchar_unlocked(c);
#else
        ::putchar(c);
#endif
    }
            
    inline void fgetline(void *data, size_t size, size_t count, FILE *stream) {
#if defined(__POSIX__)
        fread_unlocked(data, size, count, stream);
#else
        fread(data, size, count, stream);
#endif
    }
    
    
    
    constexpr const static size_t SN_ALG_FREAD_BUFFER_SIZE = 1000;
    char sn_alg_fread_buf[SN_ALG_FREAD_BUFFER_SIZE];
    char* sn_alg_fread_s = sn_alg_fread_buf + SN_ALG_FREAD_BUFFER_SIZE;
    size_t sn_alg_fread_sz = SN_ALG_FREAD_BUFFER_SIZE;

    inline char fgetchar() {
        if (sn_alg_fread_s >= sn_alg_fread_buf + SN_ALG_FREAD_BUFFER_SIZE) {
            sn_alg_fread_sz = fread_unlocked(sn_alg_fread_buf, sizeof(char), SN_ALG_FREAD_BUFFER_SIZE, stdin);
            sn_alg_fread_s = sn_alg_fread_buf;
        }
        return *(sn_alg_fread_s++);
    }
    
    inline bool fgetchar_eof() {
        return sn_alg_fread_s > sn_alg_fread_buf + sn_alg_fread_sz;
    }
    
    inline void fputchar(char c) {
        fputc_unlocked(c, stdout);   
    }
}

template <typename Tag = IOTag::Unlocked>
class IOManager {
    template <typename T>
    std::enable_if_t<detail::is_sint<T>::value, IOManager&> operator>>(T& x) {
        char c = detail::getchar(); x = 0; short f = 1;
        while (!isdigit(c)) {
            if (c == '-') f = -1;
            c = detail::getchar();
        }
        while (isdigit(c)) {
            x = (x << 3) + (x << 1) + c - 48;
            c = detail::getchar();
        }
        x *= f;
        return *this;
    }
    
    template <typename T>
    std::enable_if_t<detail::is_uint<T>::value, IOManager&> operator>>(T& x) {
        char c = detail::getchar(); x = 0;
        while (!isdigit(c)) {
            c = detail::getchar();
        }
        while (isdigit(c)) {
            x = (x << 3) + (x << 1) + c - 48;
            c = detail::getchar();
        }
        return *this;
    }

    template <typename T>
    std::enable_if_t<detail::is_sint<T>::value, IOManager&> operator<<(T& x) {
        int cnt = 0;
        char c[16];
        while (x) {
            ++cnt;
            c[cnt] = static_cast<char>((x % 10) + 48);
            x /= 10;
        }
        if (x < 0) {
            c[15] = '-';
            ++cnt;
        }			
        while (cnt) {
            detail::putchar(c[cnt]);
            --cnt;
        }
        return *this;
    }
    
    template <typename T>
    std::enable_if_t<detail::is_uint<T>::value, IOManager&> operator<<(T& x) {
        int cnt = 0;
        char c[16];
        while (x) {
            ++cnt;
            c[cnt] = static_cast<char>((x % 10) + 48);
            x /= 10;
        }
        while (cnt) {
            detail::putchar(c[cnt]);
            --cnt;
        }
        return *this;
    }

    template <typename ...Args>
    void read(const char* fmt, Args&&... args) {
        scanf(fmt, std::forward<Args>(args)...);
    }

    void readline(char* s, size_t len) {
        // gets? no way
        fgets_unlocked(s, len, stdin);
    }

    void writeline(const char* s) {
        puts(s);
        return *this;
    }
    
    template <typename ...Args>
    void write(const char* fmt, Args&&... args) {
        printf(fmt, std::forward<Args>(args)...);
    }
};

template <>
class IOManager<IOTag::FRW> {
    template <typename T>
    std::enable_if_t<detail::is_sint<T>::value, IOManager&> operator>>(T& x) {
        char c = detail::fgetchar(); x = 0; short f = 1;
        while (!isdigit(c)) {
            if (c == '-') f = -1;
            c = detail::fgetchar();
        }
        while (isdigit(c)) {
            x = (x << 3) + (x << 1) + c - 48;
            c = detail::fgetchar();
        }
        x *= f;
        return *this;
    }
    
    template <typename T>
    std::enable_if_t<detail::is_uint<T>::value, IOManager&> operator>>(T& x) {
        char c = detail::fgetchar(); x = 0;
        while (!isdigit(c)) {
            c = detail::fgetchar();
        }
        while (isdigit(c)) {
            x = (x << 3) + (x << 1) + c - 48;
            c = detail::fgetchar();
        }
        return *this;
    }

    template <typename T>
    std::enable_if_t<detail::is_sint<T>::value, IOManager&> operator<<(T& x) {
        int cnt = 0;
        char c[16];
        while (x) {
            ++cnt;
            c[cnt] = static_cast<char>((x % 10) + 48);
            x /= 10;
        }
        if (x < 0) {
            c[15] = '-';
            ++cnt;
        }			
        while (cnt) {
            detail::fputchar(c[cnt]);
            --cnt;
        }
        return *this;
    }
    
    template <typename T>
    std::enable_if_t<detail::is_uint<T>::value, IOManager&> operator<<(T& x) {
        int cnt = 0;
        char c[16];
        while (x) {
            ++cnt;
            c[cnt] = static_cast<char>((x % 10) + 48);
            x /= 10;
        }
        while (cnt) {
            detail::fputchar(c[cnt]);
            --cnt;
        }
        return *this;
    }
    
    template <typename ...Args>
    void read(const char* fmt, Args&&... args) {
        fscanf(stdin, fmt, std::forward<Args>(args)...);
    }
    
    void readline(char* arr, size_t len) {
        fgets_unlocked(arr, len, stdin);
    }
    
    void writeline(const char* s) {
        fputs_unlocked(s, stdout);
    }
    
    template <typename T>
    void write(T arr[], size_t len) {
        fwrite_unlocked(arr, sizeof(T), len, stdout);
    }
    
    // ISO C++ causes ambigious overload resolution
#if defined(SN_NO_PEDANTIC_BEHAVIOR)
    template <typename ...Args>
    void write(const char* fmt, Args&&... args) {
        fprintf(stdin, fmt, std::forward<Args>(args)...);
    }
#endif
    
    bool is_eof() {
        return detail::fgetchar_eof();
    }
};

template <>
class IOManager<IOTag::IOStream> {
    static void initIO() {
        std::ios_base::sync_with_stdio(false);
        std::cin.tie(nullptr);
    }
    
    template <typename T>
    IOManager& operator>>(T& v) {
        std::cin >> v;
        return *this;
    }
    
    template <typename T>
    IOManager& operator<<(T&& v) {
        std::cout << std::forward<T>(v);
        return *this;
    }
    
    void operator<<(const char* s) {
        std::cout.write(s, strlen(s) + 1);
        // std::fill_n(std::ostream_iterator<const char*>(std::cout, "\n"), strlen(s) + 1, s);
    }
    
    template <typename T>
    void read(T arr[], const char* delim) {
        std::copy(
            std::istream_iterator<T>(std::cin),
            std::istream_iterator<T>(),
            arr
        );
    }
    
    template <typename ...Args>
    void read(Args&&... args) {
        std::initializer_list<int>{(std::cin >> args, 0)...};
    }
    
    void readline(std::string& s) {
        std::getline(std::cin, s);
    }
    
    void writeline(const char* s) {
        std::cout << s << '\n';
    }
    
    template <typename T>
    void writeline(const char* delim, T&& v) {
        std::cout << v << '\n';
    }
    
    template <typename T, typename ...Args>
    void writeline(const char* delim, T&& v, Args&&... args) {
        std::cout << std::forward<T&&>(v) << delim;
        writeline(delim, std::forward<Args>(args)...);
    }
    
    
    template <typename ...Args>
    void write(Args&&... args) {
        std::initializer_list<int>{(std::cin << args, 0)...};
    }
    
    
    bool is_eof() {
        return std::cin.eof();
    }
};
