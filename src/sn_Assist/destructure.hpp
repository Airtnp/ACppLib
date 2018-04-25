#include "../sn_CommonHeader.h"

namespace sn_Assist {
#ifdef SN_ENABLE_CPP17_EXPERIMENTAL
    namespace sn_destructure {
        // Or not specify number, just write N structure match args
        template <std::size_t I, typename T, std::size_t ...Is>
        auto destruture_impl(T&& t, std::index_sequence<Is...>) {
            return std::forward_as_tuple(*std::next(std::begin(std::forward<T>(t)), I+Is)...);
        }

        template <std::size_t I, std::size_t Cnt, typename T>
        auto destructure(T&& t) {
            return destructure_impl<I>(std::forward<T>(t), std::make_index_sequence<Cnt>{});
        }

        template <std::size_t Cnt, typename T>
        auto destructure(T&& t) {
            return destructure_impl<0>(std::forward<T>(t), std::make_index_sequence<Cnt>{});
        }

        /*
        Usage:
            int main() {
                std::vector<int> v{1, 2, 3};
                auto [a, b] = destructure<2>(v); // forward_as_tuple ensures its reference
            }
        */
    }
#endif
}