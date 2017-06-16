#include "../sn_CommonHeader.h"

namespace sn_Meta {
    class type_id_info {
    public:
        using id_t = std::size_t;
    private:
        id_t id;
        constexpr type_id_info(id_t i = 0)
            : id(i) {}
    public:
        constexpr bool operator==(const type_id_info& rhs) const noexcept { return id == rhs.id; }
        constexpr bool operator!=(const type_id_info& rhs) const noexcept { return id != rhs.id; }
        constexpr bool before (const type_id_info& rhs) const noexcept { return id < rhs.id; }
        id_t get() const noexcept { return id; }
        std::size_t hash_code() const noexcept { return std::hash<std::size_t>()(id); }
        template <typename T>
        static const type_id_info& make() {
            return std::is_void<T>::value ? xref_void() : xref<std::remove_cv_t<T>>();
        }
    private:
        static id_t push() {
            static id_t id;
            return ++id;
        }
        template <typename T>
        static const type_id_info& xref() {
            static type_id_info info(push());
            return info;
        }
        static const type_id_info& xref_void() {
            static type_id_info info;
            return info;
        }
    };

    template <typename T>
    const type_id_info& typeid() {
        return type_id_info::make<T>();
    }
}