#ifndef SN_FUNCTION_FAILABLE_H
#define SN_FUNCTION_FAILABLE_H

#include <optional>
#include <utility>

namespace sn_Function {
#ifdef SN_ENABLE_CPP17_EXPERIMENTAL
    namespace failable {
        
        template <typename R, typename ... P>
        auto make_failable(R (*f)(P ... ps)) {
            return [f](std::optional<P> ... xs) -> std::optional<R> {
                if ((xs && ...)) {
                    return {f(*(xs)...)};
                } else {
                    return {};
                }
            };
        }

        template <typename F>
        auto make_failable(F&& f) {
            return [f = std::forward<F>(f)](auto&&... xs)
                -> std::optional<decltype(std::forward<F>(f).get()(*(std::forward<decltype(xs)>(xs))...))> {
                if ((xs && ...)) {
                    return std::forward<F>(f).get()(*(std::forward<decltype(xs)>(xs)...));
                } else {
                    return {};
                }
            }
        }

        // join
        template <typename R, typename ... P>
        auto make_failable(std::optional<R> (*f)(P ... ps)) {
            return [f](std::optional<P> ... xs) -> std::optional<R> {
                if ((xs && ...)) {
                    return f(*(xs)...);
                } else {
                    return {};
                }
            };
        }
    }
#endif
    
}



#endif