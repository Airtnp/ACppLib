#include "../sn_CommonHeader.h"

namespace sn_Assist {

#ifdef SN_ENABLE_CPP17_EXPERIMENTAL
    // TODO: nested bind/const binder/noexcept/volatile/const
    // TODO: ref: https://github.com/lhmouse/MCF/blob/master/MCF/src/Function/Bind.hpp
    namespace sn_bind {

        template <typename F, typename ...Args>
        struct Binder;

        template<std::size_t I>
        struct placeholder {
            inline static const constexpr auto value = I;
        };

        template<typename T>
        struct is_placeholder : std::false_type {};

        template<std::size_t I>
        struct is_placeholder<placeholder<I>> : std::true_type {};

        template<typename T>
        struct is_reference_wrapper : std::false_type {};

        template<typename T>
        struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type {};

        template <typename T>
        struct is_binder : std::false_type {};

        template <typename F, typename ...Args>
        struct is_binder<Binder<F, Args...>> : std::true_type {};

        template<std::size_t I, typename... Args, typename... UArgs>
        decltype(auto) bind_map(std::tuple<Args...>& args, [[maybe_unused]] std::tuple<UArgs...>& uargs)
        {
            [[maybe_unused]] auto& arg = std::get<I>(args);
            using type_t = std::tuple_element_t<I, std::tuple<Args...>>;
            using decayed_t = std::decay_t<type_t>;
            
            // 
            if constexpr (is_placeholder<decayed_t>::value) {
                return (std::get<DecayedType::value>(uargs));
            }
            else if constexpr (is_reference_wrapper<decayed_t>::value) {
                return arg.get();
            }
            else if constexpr (is_binder<decayed_t>::value) {
                return std::invoke(arg);
            }
            else {
                return arg;
            }
        }

        template<typename F, typename... Args>
        struct Binder {
            std::tuple<Args...> Parameters;
            F Callable;
            
            template<typename... UArgs>
            decltype(auto) operator()(UArgs&&... uargs) {
                // or bind_impl(std::make_integer_sequence<std::size_t, sizeof...(Args)>{}, std::forward_as_tuple(std::forward<UArgs>(uargs)...));
                return bind_impl(std::make_integer_sequence<std::size_t, sizeof...(Args)>{}, std::forward<UArgs>(uargs)...);
            }
            
            template<int... I, typename... UArgs>
            decltype(auto) bind_impl(std::integer_sequence<std::size_t, I...>, UArgs&&... uargs) {
                auto uargsTuple = std::make_tuple(std::forward<UArgs>(uargs)...);
                return std::invoke(Callable, bind_map<I>(Parameters, uargsTuple)...);
            }
        };

        template<typename F, typename... Args>
        Binder<std::decay_t<F>, std::decay_t<Args>...> bind(F&& f, Args&&... args) {
            return {{std::forward<Args>(args)...}, std::forward<F>(f)};
        }

        inline static constexpr const auto _1 = placeholder<0>{};
        inline static constexpr const auto _2 = placeholder<1>{};
    }
#endif
 
}

