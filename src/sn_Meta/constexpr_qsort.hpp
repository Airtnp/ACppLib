#include "../sn_CommonHeader.h"
#include "../sn_TypeLisp.hpp"

namespace sn_Meta {
#ifdef SN_ENABLE_CPP17_EXPERIMENTAL
    namespace constexpr_alg {

        template <template <typename ...> TL, typename ... Ts, typename ... Us>
        constexpr auto operator|(TL<Ts...>, TL<Us...>) {
            return TL<Ts..., Us...>{};
        }

        template <typename C, typename P, template <typename...> TL, typename ...Ts>
        constexpr auto partition(C&& compare, P&& pivot, TL<Ts...>&& tl) {
            if constexpr (sizeof...(Ts) == 0) {
                return std::make_pair(TL<>{}, TL<>{});
            } else {
                constexpr auto head = sn_TypeLisp::TypeCar_t<TL<Ts...>>{};
                constexpr auto right = partition(compare, pivot, sn_TypeLisp::TypeCdr_t<TL<Ts...>>{});
                if constexpr (compare(head, right)) {
                    return std::make_pair(TL<head>{} | right.first, right.second);
                } else {
                    return std::make_pair(right.first, TL<head>{} | right.second);
                }
            }
        }

        template <typename C, template <typename...> TL, typename Ts>
        constexpr auto qsort(TL<Ts...>&& tl, C&& compare) {
            if constexpr (sizeof...(Ts) == 0) {
                return TL<>{};
            } else {
                constexpr auto pivot = sn_TypeLisp::TypeCar_t<TL<Ts...>>{};
                constexpr auto right = partition(compare, pivot, sn_TypeLisp::TypeCdr_t<TL<Ts...>>{});
                return qsort(r.first, compare) | TL<decltype(pivot)>{} | qsort(r.second, compare);
            }
        }
    }
#endif
}