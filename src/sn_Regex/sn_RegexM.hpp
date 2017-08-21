#ifndef SN_REGEX_M_H
#define SN_REGEX_M_H

#include "../sn_CommonHeader.h"
#include "../sn_Meta/constexpr_control.hpp"

// ref: https://zhuanlan.zhihu.com/p/28484185
namespace sn_Regex {

    using namespace sn_constexpr::control;
    
    template <char ...Cs>
    struct char_sequence {
        template <size_t I>
        constexpr static auto get() {
            static_assert(i < sizeof...(Cs), "Access out of bound.");
            return char_constant<std::get<i>(std::make_tuple(Cs...))>{};
        }
    };

    template <typename Seq, size_t I, typename Res>
    struct parse_result {
        constexpr static auto sequence() {
            return Seq{};
        }
        constexpr static auto get() {
            return Seq::get<I>{};
        }
        constexpr static auto peek() {
            return Seq::get<I + 1>{};
        }
        constexpr static auto result() {
            return Res{};
        }
        constexpr static auto next() {
            return parse_result<Seq, I + 1, Res>{};
        }
        template <typename R>
        constexpr static auto result(R) {
            return parse_result<Seq, I, R>{};
        }
    };
    
    template <typename T>
    struct regex {};

    template <char c>
    struct match : regex<match<c>> {};

    template <char c>
    constexpr auto make_match(char_constant<c>) {
        return match<c>{};
    }

    template <char c>
    struct kleene : regex<kleene<c>> {};

    template <char c>
    constexpr auto make_kleene(char_constant<c>) {
        return kleene<c>{};
    }

    template <typename ...Rs>
    struct concat : regex<concat<Rs...>> {};

    template <typename ...Rs>
    constexpr auto make_concat(regex<Rs>...) {
        return concat<Rs...>{};
    }

    template <typename ...Rs, typename ...Ts>
    constexpr auto make_concat(concat<Rs...>, regex<Ts>...) {
        return concat<Rs..., Ts...>{};
    }

    template <typename ...Rs>
    struct alter : regex<alter<Rs...>> {};

    template <typename ...Rs>
    constexpr auto make_alter(regex<Rs>...) {
        return alter<Rs...>{};
    }

    template <typename ...Rs, typename ...Ts>
    constexpr auto make_alter(alter<Rs...>, regex<Ts>...) {
        return alter<Rs..., Ts...>{};
    }

    template<typename Res>
    constexpr static auto parse_kleene(Res r) {
        auto token = r.get();
        auto next = r.peek();
        return cond(
            next == char_constant<'*'>{},
            [=] { 
                return r.forward().forward().result(make_kleene(token)); 
            },
            [=] { 
                return r.forward().result(make_match(token)); 
            }
        )();
    }

    template<class Res>
    constexpr static auto parse_concatination(Res r)
    {
        return iter(
            parse_kleene(r),
            [](auto res) {
                return (res.get() != char_constant<'\0'>{}) && (res.get() != char_constant<'|'>{});
            },
            [](auto res) {
                auto e = parse_kleene(res);
                return e.result(make_concat(res.result(), e.result()));
            }
        );
        /* 相当于
        auto regex = make_concat(parse_kleene(r));
        static_for (;;) {
            if (r.get() ！= '\0' && r.get() != '|') {
                regex = make_concat(regex, parse_kleene(r.forward()));
            }
            else {
                return regex;
            }
        }
        */
    }


    struct regex_parser {
        template <typename Seq>
        constexpr static auto parse(Seq s) {
            return parse_alternative(parse_result<Seq, 0, void>{});
        }
    private:
        template <typename Res>
        constexpr static auto parse_alternative(Res r) {
            return iter(
                parse_concatination(r),
                [](auto res) {
                    return res.get() != char_constant<'\0'>{};
                },
                [](auto res) {
                    static_assert((res.get() == char_constant<'|'>{})::value);
                    auto e = parse_concatination(res.forward());
                    return e.result(make_alter(res.result(), e.result()));
                }
            );
        }

#if defined(__GNUC__)
        template<typename TChar, TChar... chars>
        constexpr decltype(auto) operator"" _regex() {
            return regex_parser::parse(char_sequence<chars..., '\0'>{}).result();
        }
#endif
    };

}


#endif