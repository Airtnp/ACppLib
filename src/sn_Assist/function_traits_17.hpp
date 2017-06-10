#include "../sn_CommonHeader.h"

namespace sn_Assist {
#ifdef SN_ENABLE_CPP17_EXPERIMENTAL
    namespace function_traits {
        template <typename Sig>
        struct function_traits_impl;

        template <typename R, typename ...Args>
        struct function_traits_impl<R(Args...)> {
            using result_type = R;
            using function_type = R(Args...);
            constexpr static auto arity = sizeof...(Args);
            using args_type = std::tuple<Args...>;
        };

        template <typename Sig>
        struct function_traits_impl<std::function<Sig>>
            : function_traits_impl<Sig> {};

        template <typename F>
        struct function_traits
            : function_traits_impl<
                decltype(std::function{std::declval<F>()})
            > {};

        template <typename ...Args>
        struct function_params {};

        template <typename R, template <typename ...> class Arg, bool Noexcept = false, bool Pointer = false, bool Lambda = false>
        struct function_signature {};

        template <typename R, typename ...Args, bool Noexcept, bool Pointer, bool Lambda>
        struct function_signature<R, function_params<Args...>, Noexcept, Pointer, Lambda> {
            using result_type = R;
            using args_type = function_params<Args...>;
            using function_type = R(Args...);
            constexpr static const auto arity = sizeof...(Args);
            constexpr static const bool is_noexcept = Noexcept;
            constexpr static const bool is_pointer = Pointer;
            constexpr static const bool is_lambda = Lambda;
        };

        template <typename R, typename ...Args>
        function_signature(R(*f)(Args...)) -> function_signature<R, function_params<Args...>>;

        template <typename R, typename ...Args>
        function_signature(R(*f)(Args...) noexcept) -> function_signature<R, function_params<Args...>, true>;
        
        template <typename R, typename C, typename ...Args>
        function_signature(R(C::*f)(Args...)) -> function_signature<R, function_params<C, Args...>>;
        
        template <typename R, typename C, typename ...Args>
        function_signature(R(C::*f)(Args...) volatile) -> function_signature<R, function_params<volatile C&, Args...>>;
        
        // ...

        template <typename F>
        function_signature(F&&) -> function_signature<
            typename decltype(function_signature(&std::decay_t<F>::operator()))::result_type
            typename decltype(function_signature(&std::decay_t<F>::operator()))::args_type
            decltype(function_signature(&std::decay_t<F>::operator())))::is_noexcept,
            false, 
            true
        >;
    }

#endif
}