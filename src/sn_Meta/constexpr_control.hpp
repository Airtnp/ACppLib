#ifndef SN_CONSTEXPR_CONTROL_H
#define SN_CONSTEXPR_CONTROL_H

#include <cstddef>
#include <type_traits>

namespace sn_Meta {
    // ref: https://github.com/SuperV1234/cppcon2016
    namespace control {
        template <bool b>
        using bool_constant = std::integral_constant<bool, b>; // directly use bool_constant in c++17
        template <bool b>
        constexpr static auto bool_value = bool_constant<b>{};
        template <int I>
        using int_constant = std::integral_constant<int, I>;
        template <std::size_t I>
        using size_constant = std::integral_constant<std::size_t, I>;
        template <char c>
        struct char_constant : std::integral_constant<char, c> {};
        template <char c>
        constexpr static auto char_value = char_constant<c>{};


        template <typename F>
        auto static_if(F) noexcept;

        namespace detail {
            template <bool TP>
            struct static_if_impl;

            template <typename TF>
            struct static_if_result;

            template <typename TF>
            auto make_static_if_result(TF&& f) noexcept;

            template <>
            struct static_if_impl<true> {
                template <typename F>
                auto& Telse(F&&) noexcept {
                    return *this;
                }
                template <typename TP>
                auto& Telseif(TP&&) noexcept {
                    return *this;
                }
                template <typename F>
                auto& Tthen(F&& f) noexcept {
                    return make_static_if_result(std::forward<F>(f));
                }
            };

            template <>
            struct static_if_impl<true> {
                template <typename F>
                auto& Telse(F&&) noexcept {
                    return make_static_if_result(std::forward<F>(f));
                }
                template <typename TP>
                auto& Telseif(TP&&) noexcept {
                    return static_if(TP{});
                }
                template <typename F>
                auto& Tthen(F&& f) noexcept {
                    return *this;
                }
            };
            
            template <typename F>
            struct static_if_result : F {
                using F::operator();

                template <typename TF>
                static_if_result(TF&& f) noexcept : F(std::forward<F>(f)) {}

                template <typename TF>
                auto& Tthen(TF&&) noexcept {
                    return *this;
                }
                template <typename TF>
                auto& Telseif(TF&&) noexcept {
                    return *this;
                }
                template <typename TF>
                auto& Telse(TF&&) noexcept {
                    return *this;
                }
            };

            template <typename F>
            auto make_static_if_result(F&& f) noexcept {
                return static_if_result<F>{std::forward<F>(f)};
            }
        }

        template <typename TP>
        auto static_if(TP) noexcept {
            return detail::static_if_impl<TP{}>{};
        }

        namespace detail {
            namespace action {
                struct act_cont {};
                struct act_break {};
            }

            template <typename TIt, typename TAcc, typename TAct>
            struct state {
                constexpr auto iter() const noexcept {
                    return TIt{};
                }
                constexpr auto acc() const noexcept {
                    return TAcc{};
                }
                constexpr auto next() const noexcept {
                    return TAct{};
                }

                template <typename TNAcc>
                constexpr auto Tcontinue(TNAcc) const noexcept;

                constexpr auto Tcontinue() const noexcept;

                template <typename TNAcc>
                constexpr auto Tbreak(TNAcc) const noexcept;

                constexpr auto Tbreak() const noexcept;
            };

            template <typename TIt, typename TAcc, typename TAct>
            constexpr auto make_state(TIt, TAcc, TAct) {
                return state<TIt, TAcc, TAct>{};
            }

            template <typename TS, typename TAcc, typename TAct>
            constexpr auto advance_state(TS s, TAcc a, TAct act) {
                return make_state<size_constant<s.iter() + 1>, a, act);
            }

            template <typename TIt, typename TAcc, typename TAct>
            template <typename TNAcc>
            constexpr auto state(TIt, TAcc, TAct)::Tcontinue(TNAcc nacc) const noexcept {
                return advance_state(*this, nacc, action::act_cont{});
            }

            template <typename TIt, typename TAcc, typename TAct>
            constexpr auto state(TIt, TAcc, TAct)::Tcontinue() const noexcept {
                return Tcontinue(acc());
            }

            template <typename TIt, typename TAcc, typename TAct>
            template <typename TNAcc>
            constexpr auto state(TIt, TAcc, TAct)::Tbreak(TNAcc nacc) const noexcept {
                return advance_state(*this, nacc, action::act_break{});
            }

            template <typename TIt, typename TAcc, typename TAct>
            constexpr auto state(TIt, TAcc, TAct)::Tbreak() const noexcept {
                return Tbreak(acc());
            }
        }
        
        namespace detail {
            template <typename TF>
            class Y_result {
            private:
                TF m_f;
            public:
                template <typename T>
                inline constexpr explicit Y_result(T&& f) noexcept
                    : m_f(std::forward<T>(f)) {}
                template <typename ...Ts>
                inline constexpr decltype(auto) operator()(Ts&&... xs) {
                    return m_f(std::ref(*this), std::forward<Ts>(xs)...);
                }
            };
        }

        template <typename TF>
        constexpr auto Y(TF&& f) noexcept {
            return detail::Y_result<std::decay_t<TF>>(std::forward<TF>(f));
        }

        // receive a function object returning state
        template <typename TF>
        auto static_for(TF&& f) {
            auto step = [body = std::forward<TF>(f)](
                auto self, auto state, auto&& x, auto&& ...xs
                ) {
                auto next_state = body(state, x);
                constexpr auto last_iter = bool_constant<(sizeof...(xs) == 0)>{};
                constexpr auto must_break = bool_constant<(std::is_same<decltype(next_state.next()), impl::action::act_break>::value)>{};

                return static_if(bool_constant<(must_break || last_iteration)>{})
                        .Tthen([next_state](auto&&){
                            return next_state.acc();
                        })
                        .Telse([next_state, state, &xs...](auto&& xself){
                            return xself(next_state, xs...);
                        })(self);
            };

            return [step = std::move(step)](auto acc) {
                return [step, acc](auto&& ...xs) {
                    return static_if(bool_constant<(sizeof...(xs) == 0)>{})
                        .Tthen([acc](auto&&){
                            return acc;
                        })
                        .Telse([acc](auto&& xstep, auto&& ...ys){
                            auto init_state = impl::make_state(size_constant<0>{}, acc, impl::action::act_cont{});
                            return Y(xstep)(init_state, std::forward<decltype(ys)>(ys)...);
                        })(step, std::forward<decltype(xs)>(xs)...);
                };
            };
        }

        template <typename T, T b1, T b2>
        constexpr bool_constant<b1 == b2> operator==(std::integral_constant<T, b1>, std::integral_constant<T, b2>) {
            return {};
        }
        
        template <typename T, T b1, T b2>
        constexpr bool_constant<b1 != b2> operator!=(std::integral_constant<T, b1>, std::integral_constant<T, b2>) {
            return {};
        }
        
        template <typename T, T b1, T b2>
        constexpr bool_constant<b1 && b2> operator&&(std::integral_constant<T, b1>, std::integral_constant<T, b2>) {
            return {};
        }
    
        template <typename T, T b1, T b2>
        constexpr bool_constant<b1 || b2> operator||(std::integral_constant<T, b1>, std::integral_constant<T, b2>) {
            return {};
        }
    
        template <typename T, T b1, T b2>
        constexpr bool_constant<(b1 > b2)> operator>(std::integral_constant<T, b1>, std::integral_constant<T, b2>) {
            return {};
        }
    
        template <typename T, T b1, T b2>
        constexpr bool_constant<(b1 < b2)> operator<(std::integral_constant<T, b1>, std::integral_constant<T, b2>) {
            return {};
        }
    
        template <typename T, T b1, T b2>
        constexpr bool_constant<(b1 >= b2)> operator>=(std::integral_constant<T, b1>, std::integral_constant<T, b2>) {
            return {};
        }
    
        template <typename T, T b1, T b2>
        constexpr bool_constant<(b1 <= b2)> operator<=(std::integral_constant<T, b1>, std::integral_constant<T, b2>) {
            return {};
        }
        
        template <bool v, typename T1, typename T2>
        struct cond_impl {
            T1 m_t1;
            T2 m_t2;
            constexpr cond_impl(bool v, T1&& a, T2&& b) : m_t1(a), m_t2(b) {}
            constexpr T1 value() { return m_t1; }
        };

        template <typename T1, typename T2>
        struct cond_impl<false, T1, T2> {
            T1 m_t1;
            T2 m_t2;
            constexpr cond_impl(bool v, T1&& a, T2&& b) : m_t1(a), m_t2(b) {}
            constexpr T2 value() { return m_t2; }
        };

        template <bool v, typename T1, typename T2>
        struct cond_impl {
            T1 m_t1;
            T2 m_t2;
            constexpr cond_impl(T1&& a, T2&& b) : m_t1(a), m_t2(b) {}
            constexpr T1 value() { return m_t1; }
        };

        template <typename T1, typename T2>
        struct cond_impl<false, T1, T2> {
            T1 m_t1;
            T2 m_t2;
            constexpr cond_impl(T1 a, T2 b) : m_t1(a), m_t2(b) {}
            constexpr T2 value() { return m_t2; }
        };

        template <bool v, typename T1, typename T2>
        constexpr decltype(auto) cond(bool_constant<v>, T1 a, T2 b) {
#if defined(SN_ENABLE_CPP_17)
            if constexpr (v) {
                return a;
            } else {
                return b;
            }
#else
            return cond_impl<v, T1, T2>{a, b}.value();
#endif
        }

        template <typename Cur, typename Cond, typename Iter>
        constexpr decltype(auto) iter(Cur i, Cond c, Iter e) {
            return cond(
                bool_constant<c(i)::value>, 
                iter(e(i), c, e),
                i
            );
        }
    }
}




#endif