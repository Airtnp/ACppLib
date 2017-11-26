#ifndef SN_COROUTINE_H
#define SN_COROUTINE_H

#include "sn_CommonHeader.h"

namespace sn_Coroutine {
#define SN_COROUTINE_GEN_COND(N) \
    case N: \
        m_result = static_cast<At_t<N>&>(*this).generate(m_result) \
        ++m_state; \
        return m_result


    template <typename R, typename ...Ts>
    class Generator {
        size_t m_state;
        template <size_t I>
        using At_t = TypeAt<TypeList<std::decay_t<Ts>...>, I>;
        // using R = std::invoke_result_t<At_t<0>::generate, R>;
        R m_result;

        // template <size_t I>
        // using R_t = typename std::invoke_result<TypeAt<TypeList<std::decay_t<Ts>...>, I>::generate, At_t<I>*, std::declval<Rt_t<I-1>>()>::type; 
    public:
        Generator() : m_state{0} {}
        R next() {
            switch(m_state) {
                for(;;) {
                    case sizeof...(Ts):
                        m_state = 0;
                        goto defend;
                    SN_COROUTINE_GEN_COND(0);
                    SN_COROUTINE_GEN_COND(1);
                    SN_COROUTINE_GEN_COND(2);
                    SN_COROUTINE_GEN_COND(3);
                    SN_COROUTINE_GEN_COND(4);
                    SN_COROUTINE_GEN_COND(5);
                    SN_COROUTINE_GEN_COND(6);
                    SN_COROUTINE_GEN_COND(7);
                    default:
                        defend: ;
                }
            }
        }
    };  

    class Fibonacci : public Generator<size_t, Fibonacci> {
        size_t a, b;
    public:
        using R = size_t;
        Fibonacci() : a{0}, b{1} {}
        size_t generate(size_t& prev) {
            size_t c = a + b;
            a = b;
            b = c;
            return c;
        }
    };
}

#endif