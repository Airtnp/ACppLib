#ifndef SN_TYPE_MEMOIZE_H
#define SN_TYPE_MEMOIZE_H

#include <utility>
#include <unordered_map>
#include <type_traits>
#include <tuple>

namespace sn_Type {

    namespace memoize {

        template <typename T, T t, template <typename, typename> class TCacheContainer>
        class Memoize;
        
        template <typename TValue, typename ...TArgs, TValue (*f)(TArgs...), template <typename, typename> class TCacheContainer = std::unordered_map>
        class Memoize<TValue(*)(TArgs...), f, TCacheContainer> {
            using TKey = std::tuple<typename std::decay<TArgs>::type...>;

            static_assert(std::is_same_v<TKey, TCacheContainer::key_type>, "TCacheContainer must have same key type");
            static_assert(std::is_same_v<TValue, TCacheContainer::mapped_type>, "TCacheContainer must have same key type");

            static TValue call(TArgs&&... args) {
                static TCacheContainer<TKey, TValue> cache;
                auto key = TKey{std::forward<TArgs>(args)...};
                auto res = cache.find(key);
                if (res != cache.end()) {
                    return *res;
                } else {
                    auto value = f(std::forward<TArgs>(args)...);
                    cache.insert(std::make_pair<TKey, TValue>(key, value));
                    return value;
                }
            }
        };
    }
}

#endif