#ifndef SN_BUILTIN_MEM_H
#define SN_BUILTIN_MEM_H

#include <type_traits>
#include <memory>

namespace sn_Builtin {
    namespace mem {
        template <typename T>
        inline T* trivially_copy_n(const T* src, size_t n, T* res) noexcept {
            static_assert(std::is_trivial<T>{}, "Non-trivial type");
            static_assert(std::is_copy_assignable<T>{}, "Type should be copy assignable");

            std::memcpy(res, src, sizeof(T) * n);
            return res + n;
        }

        template <typename T>
        inline T* trivially_copy(const T* first, const T* last, T* res) noexcept {
            return trivially_copy_n(first, last - first, res);
        }
    }
}



#endif