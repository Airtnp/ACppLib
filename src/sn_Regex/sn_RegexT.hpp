#ifndef SN_REGEX_T_H
#define SN_REGEX_T_H

#include "../sn_CommonHeader.h"

// ref: https://gist.github.com/timshen91/f6a3f040e5b5b0685b2a
// Compare to parserc/type-calculus
// TODO: ref boost.Xpressive
namespace sn_RegexT {
	template <typename L, typename R>
	struct ConcatExpr;

	template <typename L, typename R>
	struct AltExpr;

	template <typename SubExpr>
	struct RepExpr;

	template <char ch>
	struct SymbolExpr;

	struct EmptyExpr;
	using EpsilonExpr = EmptyExpr;

	template <typename SubExpr>
	using OptionalExpr = AltExpr<SubExpr, EpsilonExpr>;

	template <typename RegExpr>
	struct Eval;

	template <typename L, typename R>
	struct Eval<AltExpr<L, R>> {
		template <typename Continuation>
		static bool Apply(const char* target, Continuation cont) {
			return Eval<L>::Apply(target, cont) || Eval<R>::Apply(target, cont);
		}
	};

	template <typename L, typename R>
	struct Eval<ConcatExpr<L, R>> {
		template <typename Continuation>
		static bool Apply(const char* target, Continuation cont) {
			return Eval<L>::Apply(target, [cont](const char* rest) -> bool {
				return Eval<R>::Apply(rest, cont);
			});
		}
	};

	template <typename SubExpr>
	struct Eval<RepExpr<SubExpr>> {
		template <typename Continuation>
		static bool Apply(const char* target, Continuation cont) {
			return Eval<SubExpr>::Apply(target, [target, cont](const char* rest) -> bool {
				return target < rest && Eval<RepExpr<SubExpr>>::Apply(rest, cont);
			}) || cont(target);
		}
	};

	template <char ch>
	struct Eval<SymbolExpr<ch>> {
		template <typename Continuation>
		static bool Apply(const char* target, Continuation cont) {
			return *target && *target == ch && cont(target + 1);
		}
	};

	template <>
	struct Eval<EmptyExpr> {
		template <typename Continuation>
		static bool Apply(const char* target, Continuation cont) {
			return cont(target);
		}
	};

	template <typename RegExpr>
	constexpr bool RegexMatch(const char* target) {
		return Eval<RegExpr>::Apply(target, [](const char* rest) -> bool {
			return *rest == '\0';
		});
	}

	template <typename RegExpr>
	constexpr bool RegexSearch(const char* target) {
		return Eval<RegExpr>::Apply(target, [](const char* rest) -> bool {
			return true;
		}) || (*target && RegexSearch<RegExpr>(target + 1));
	}

	// assert((RegexMatch<ConcatExpr<MatchExpr<'a'>, MatchExpr<'b'>>>("ab")));

}







#endif