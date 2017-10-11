#ifndef SN_FUNCTION_MEMOIZE_H
#define SN_FUNCTION_MEMOIZE_H

#include <utility>

namespace sn_Function {
    namespace memoize {
        template <typename T, T t, template <typename, typename> class TCacheContainer>
        class Memoize;
        
        template <typename TValue, typename ...TArgs, TValue (*f)(TArgs...), template <typename, typename> class TCacheContainer>
        class Memoize<TValue(*)(TArgs...), f, TCacheContainer> {
            using TKey = std::tuple<typename std::decay<TArgs>::type...>;
            static TValue call(TArgs&&... args) {
                static TCacheContainer<TKey, TValue> cache;
                auto key = TKey{std::forward<TArgs>(args)...};
                auto res = cache.get(key);
                if (res != nullptr) {
                    return *res;
                } else {
                    auto value = f(std::forward<TArgs>(args)...);
                    cache.set(key, value);
                    return value;
                }
            }
        };
    }
}


#endif