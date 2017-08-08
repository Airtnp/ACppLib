#ifndef SN_TYPETRAITS_EXPRESSION_H
#define SN_TYPETRAITS_EXPRESSION_H

#include <type_traits>
#include <utility>

namespace sn_TypeTraits {
    namespace expression {
        template <typename F, typename A>
        constexpr auto is_constant_expression_impl(F&& f, A&& a)
            -> decltype((std::forward<F>(f)(std::forward<A>(a)), std::true_type{})) {
            return {};
        }

        constexpr std::false_type is_constant_expression_impl(...) {
            return {};
        }

#define SN_TYPETRAITS_IS_CONSTANT_EXPRESSION(cls, expr, ...) \
    sn_TypeTraits::expression::is_constant_expression_impl( \
        [](auto ty) -> std::void_t<(decltype(ty)::expr(__VA_ARGS__), 0)>{}, \
        cls{} \
    ); \

        template <typename C, typename R, typename ...Args>
        constexpr auto is_constant_expression(R(C::*p)(Args...), Args&&... args) {
            return is_constant_expression_impl(
                [](auto ty)
                     -> std::void_t<(p(__VA_ARGS__), 0)>{},
                C{}
            );
        }
    }
}



#endif