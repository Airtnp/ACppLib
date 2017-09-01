#ifndef SN_ASSIST_INFIX_ADAPTOR
#define SN_ASSIST_INFIX_ADAPTOR

#include <utility>

namespace sn_Assist {
    namespace infix_adaptor {
        // TODO: for functor and &/&&/const suffix
        // TODO: F must be binary callable (concept)
        template <typename F>
        struct infix_adaptor {
            constexpr infix_adaptor() : m_func(F()) {}
            constexpr infix_adaptor(F func) : m_func(std::move(func)) {}
            // TODO: use friend and forward declaration
            F m_func;
        };

        template <typename F, typename T>
        struct infix_result {
            constexpr infix_result(F&& func, T left) : m_func(std::move(func)), m_left(std::move(left)) {}
            F m_func;
            T m_left;
        };

        template <typename F, typename T>
        constexpr infix_result<F, T> operator<(const T& left, infix_adaptor<F> infix) {
            return infix_result<F, T>{std::move(infix.m_func), left};
        }

        template <typename F, typename T, typename U>
        auto operator>(infix_result<F, T>&& res, const U& right)
            -> decltype(res.m_func(res.m_left, right)) {
            return res.m_func(res.m_left, right);
        }

    }
}




#endif