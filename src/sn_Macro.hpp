#ifndef SN_MACRO_H
#define SN_MACRO_H

#include "sn_CommonHeader.h"

namespace sn_Macro { //useless namespace

	//solve VS2015 macro expansion bugs

#define MACRO_EXPAND(...) __VA_ARGS__
#define APPLY_VARIADIC_MACRO(macro, ...) MACRO_EXPAND(macro(__VA_ARGS__))

#define MACRO_CONCAT_IMPL(A, B) A##_##B
#define MACRO_CONCAT(A, B) MACRO_CONCAT_IMPL(A, B)

#define MACRO_RAW_CONCAT_IMPL(A, B) A##B
#define MACRO_RAW_CONCAT(A, B) MACRO_RAW_CONCAT_IMPL(A, B)

// Or __COUNTER__
#define ANONYMOUS_VARIABLE(str) MACRO_RAW_CONCAT(str, __LINE__)

	// static char THIS_FILE[] = __FILE__
#define DEBUG_NEW(THIS_FILE, __FILE__)

	//tricky get va_list size (n = N - 1)
#define SN_MAX_ARG_N = 10

#define SN_REVERSE_SEQ_N() \
	20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10 \
	10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0

#define SN_SEQ_N( \
	_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
	_11, _12, _13, _14, _15, _16, _17, _18, _19, _20, N, ...) N

#define SN_GET_ARG_IMPL(...) MACRO_EXPAND(SN_SEQ_N(__VA_ARGS__))
#define SN_GET_ARG_N(...) SN_GET_ARG_IMPL(__VA_ARGS__, SN_REVERSE_SEQ_N())

#define SN_POP_ARG_IMPL(a, ...) __VA_ARGS__

#define SN_POP_ARG_0(...) MACRO_EXPAND(__VA_ARGS__)
#define SN_POP_ARG_1(...) MACRO_EXPAND(SN_POP_ARG_IMPL(__VA_ARGS__))
#define SN_POP_ARG_2(...) SN_POP_ARG_1(SN_POP_ARG_1(__VA_ARGS__))
#define SN_POP_ARG_3(...) SN_POP_ARG_1(SN_POP_ARG_2(__VA_ARGS__))
#define SN_POP_ARG_4(...) SN_POP_ARG_1(SN_POP_ARG_3(__VA_ARGS__))
#define SN_POP_ARG_5(...) SN_POP_ARG_1(SN_POP_ARG_4(__VA_ARGS__))
#define SN_POP_ARG_6(...) SN_POP_ARG_1(SN_POP_ARG_5(__VA_ARGS__))
#define SN_POP_ARG_7(...) SN_POP_ARG_1(SN_POP_ARG_6(__VA_ARGS__))
#define SN_POP_ARG_8(...) SN_POP_ARG_1(SN_POP_ARG_7(__VA_ARGS__))
#define SN_POP_ARG_9(...) SN_POP_ARG_1(SN_POP_ARG_8(__VA_ARGS__))

#define SN_POP_ARG_N(N, ...) APPLY_VARIADIC_MACRO(MACRO_CONCAT(SN_POP_ARG, N), __VA_ARGS__)

#define SN_TOP_ARG_IMPL(a, ...) a

#define SN_TOP_ARG_0(...) 
#define SN_TOP_ARG_1(...) MACRO_EXPAND(SN_TOP_ARG_IMPL(__VA_ARGS__))
#define SN_TOP_ARG_2(...) SN_TOP_ARG_1(SN_POP_ARG_1(__VA_ARGS__))
#define SN_TOP_ARG_3(...) SN_TOP_ARG_1(SN_POP_ARG_2(__VA_ARGS__))
#define SN_TOP_ARG_4(...) SN_TOP_ARG_1(SN_POP_ARG_3(__VA_ARGS__))
#define SN_TOP_ARG_5(...) SN_TOP_ARG_1(SN_POP_ARG_4(__VA_ARGS__))
#define SN_TOP_ARG_6(...) SN_TOP_ARG_1(SN_POP_ARG_5(__VA_ARGS__))
#define SN_TOP_ARG_7(...) SN_TOP_ARG_1(SN_POP_ARG_6(__VA_ARGS__))
#define SN_TOP_ARG_8(...) SN_TOP_ARG_1(SN_POP_ARG_7(__VA_ARGS__))
#define SN_TOP_ARG_9(...) SN_TOP_ARG_1(SN_POP_ARG_8(__VA_ARGS__))

#define SN_TOP_ARG_N(N, ...) APPLY_VARIADIC_MACRO(MACRO_CONCAT(SN_TOP_ARG, N), __VA_ARGS__)

	// SN_APPLY_NEST_N(first, last, nest, x) -> first(x, first(nest(xx), first(nest(nest(xx)), last(nest(nest(nest(xx)))))))
#define SN_APPLY_NEST_0(F, L, N, ...)

#define SN_APPLY_NEST_1(F, L, N, ...) MACRO_EXPAND(L(MACRO_EXPAND(__VA_ARGS__)))
#define SN_APPLY_NEST_2(F, L, N, ...) MACRO_EXPAND(F(MACRO_EXPAND(__VA_ARGS__), SN_APPLY_NEST_1(F, L, N, N(MACRO_EXPAND(__VA_ARGS__)))))
#define SN_APPLY_NEST_3(F, L, N, ...) MACRO_EXPAND(F(MACRO_EXPAND(__VA_ARGS__), SN_APPLY_NEST_2(F, L, N, N(MACRO_EXPAND(__VA_ARGS__)))))
#define SN_APPLY_NEST_4(F, L, N, ...) MACRO_EXPAND(F(MACRO_EXPAND(__VA_ARGS__), SN_APPLY_NEST_3(F, L, N, N(MACRO_EXPAND(__VA_ARGS__)))))
#define SN_APPLY_NEST_5(F, L, N, ...) MACRO_EXPAND(F(MACRO_EXPAND(__VA_ARGS__), SN_APPLY_NEST_4(F, L, N, N(MACRO_EXPAND(__VA_ARGS__)))))
#define SN_APPLY_NEST_6(F, L, N, ...) MACRO_EXPAND(F(MACRO_EXPAND(__VA_ARGS__), SN_APPLY_NEST_5(F, L, N, N(MACRO_EXPAND(__VA_ARGS__)))))
#define SN_APPLY_NEST_7(F, L, N, ...) MACRO_EXPAND(F(MACRO_EXPAND(__VA_ARGS__), SN_APPLY_NEST_6(F, L, N, N(MACRO_EXPAND(__VA_ARGS__)))))
#define SN_APPLY_NEST_8(F, L, N, ...) MACRO_EXPAND(F(MACRO_EXPAND(__VA_ARGS__), SN_APPLY_NEST_7(F, L, N, N(MACRO_EXPAND(__VA_ARGS__)))))
#define SN_APPLY_NEST_9(F, L, N, ...) MACRO_EXPAND(F(MACRO_EXPAND(__VA_ARGS__), SN_APPLY_NEST_8(F, L, N, N(MACRO_EXPAND(__VA_ARGS__)))))

#define SN_APPLY_NEST_N(N, F, L, NF, ...) APPLY_VARIADIC_MACRO(MACRO_CONCAT(SN_APPLY_NEST, N), F, L, NF, __VA_ARGS__)

#define SN_APPLY_MULTI_0(F) 

#define SN_APPLY_MULTI_1(F) F(1)
#define SN_APPLY_MULTI_2(F) SN_APPLY_MULTI_1(F) F(2)
#define SN_APPLY_MULTI_3(F) SN_APPLY_MULTI_2(F) F(3)
#define SN_APPLY_MULTI_4(F) SN_APPLY_MULTI_3(F) F(4)
#define SN_APPLY_MULTI_5(F) SN_APPLY_MULTI_4(F) F(5)
#define SN_APPLY_MULTI_6(F) SN_APPLY_MULTI_5(F) F(6)
#define SN_APPLY_MULTI_7(F) SN_APPLY_MULTI_6(F) F(7)
#define SN_APPLY_MULTI_8(F) SN_APPLY_MULTI_7(F) F(8)
#define SN_APPLY_MULTI_9(F) SN_APPLY_MULTI_8(F) F(9)

#define SN_APPLY_MULTI_N(N, F) APPLY_VARIADIC_MACRO(MACRO_CONCAT(SN_APPLY_MULTI, N), F)

#define SN_APPLY_REPEAT_0(F, L, ...)

#define SN_APPLY_REPEAT_1(F, L, ...) MACRO_EXPAND(L(1, MACRO_EXPAND(__VA_ARGS__)))
#define SN_APPLY_REPEAT_2(F, L, ...) SN_APPLY_REPEAT_1(F, L, MACRO_EXPAND(__VA_ARGS__)) MACRO_EXPAND(F(2, MACRO_EXPAND(__VA_ARGS__)))
#define SN_APPLY_REPEAT_3(F, L, ...) SN_APPLY_REPEAT_2(F, L, MACRO_EXPAND(__VA_ARGS__)) MACRO_EXPAND(F(3, MACRO_EXPAND(__VA_ARGS__)))
#define SN_APPLY_REPEAT_4(F, L, ...) SN_APPLY_REPEAT_3(F, L, MACRO_EXPAND(__VA_ARGS__)) MACRO_EXPAND(F(4, MACRO_EXPAND(__VA_ARGS__)))
#define SN_APPLY_REPEAT_5(F, L, ...) SN_APPLY_REPEAT_4(F, L, MACRO_EXPAND(__VA_ARGS__)) MACRO_EXPAND(F(5, MACRO_EXPAND(__VA_ARGS__)))
#define SN_APPLY_REPEAT_6(F, L, ...) SN_APPLY_REPEAT_5(F, L, MACRO_EXPAND(__VA_ARGS__)) MACRO_EXPAND(F(6, MACRO_EXPAND(__VA_ARGS__)))
#define SN_APPLY_REPEAT_7(F, L, ...) SN_APPLY_REPEAT_6(F, L, MACRO_EXPAND(__VA_ARGS__)) MACRO_EXPAND(F(7, MACRO_EXPAND(__VA_ARGS__)))
#define SN_APPLY_REPEAT_8(F, L, ...) SN_APPLY_REPEAT_7(F, L, MACRO_EXPAND(__VA_ARGS__)) MACRO_EXPAND(F(8, MACRO_EXPAND(__VA_ARGS__)))
#define SN_APPLY_REPEAT_9(F, L, ...) SN_APPLY_REPEAT_8(F, L, MACRO_EXPAND(__VA_ARGS__)) MACRO_EXPAND(F(9, MACRO_EXPAND(__VA_ARGS__)))

#define SN_APPLY_REPEAT_N(N, F, ...) APPLY_VARIADIC_MACRO(MACRO_CONCAT(SN_APPLY_REPEAT, N), F, F, MACRO_EXPAND(__VA_ARGS__))
#define SN_APPLY_REPEATEX_N(N, F, L, ...) APPLY_VARIADIC_MACRO(MACRO_CONCAT(SN_APPLY_REPEAT, N), F, L, MACRO_EXPAND(__VA_ARGS__))



#define SN_CLONE_L(N, a, ...) a
#define SN_CLONE_F(N, a, ...) MACRO_EXPAND(__VA_ARGS__) a
#define SN_CLONE_N(N, a, ...) SN_APPLY_REPEATEX_N(N, SN_CLONE_F, SN_CLONE_L, a, MACRO_EXPAND(__VA_ARGS__))

#define SN_REVERSE_F(a, ...) MACRO_EXPAND(...), SN_TOP_ARG_1(a)
#define SN_REVERSE(...) SN_APPLY_NEST_N(SN_GET_ARG_N(__VA_ARGS__), SN_REVERSE_F, MACRO_EXPAND, SN_POP_ARG_1, MACRO_EXPAND(__VA_ARGS__))

#define SN_FIRST_F(a, ...) SN_TOP_ARG_1(a), MACRO_EXPAND(__VA_ARGS__)
#define SN_FIRST_N(N, ...) SN_APPLY_NEST_N(N, SN_FIRST_F, SN_TOP_ARG_1, SN_POP_ARG_1, MACRO_EXPAND(__VA_ARGS__))


#define SN_INC(N) SN_GET_ARG_N(SN_CLONE_N(N, -, ,), -)
#define SN_DEC(N) SN_GET_ARG_N(SN_POP_ARG_1(SN_CLONE_N(N, -, ,)))

#define SN_ADD(N1, N2) SN_GET_ARG_N(SN_CLONE_N(N1, -, ,), SN_CLONE_N(N2, -, ,))
#define SN_SUB(N1, N2) SN_GET_ARG_N(SN_POP_ARG_N(N2, SN_CLONE_N(N2, -, ,)))
#define SN_NEG(N) SN_SUB(SN_MAX_ARG_N, N)

// MAX - N1 > MAX - N2 ?
#define SN_LESS_IMPL(NUM, Y, N) SN_REVERSE(SN_CLONE_N(NUM, N, ,), SN_CLONE_N(SN_NEG(NUM), Y, ,))
#define SN_MORE_IMPL(NUM, Y, N) SN_REVERSE(SN_CLONE_N(NUM, Y, ,), SN_CLONE_N(SN_NEG(NUM), N, ,))

#define SN_LESS(N1, N2, Y, N) SN_GET_ARG_IMPL(SN_CLONE_N(N2, -, ,), SN_LESS_IMPL(N1, Y, N))
#define SN_MORE(N1, N2, Y, N) SN_GET_ARG_IMPL(SN_CLONE_N(N2, -, ,), SN_MORE_IMPL(N1, Y, N))
#define SN_EQN(N1, N2, Y, N) SN_MORE(N1, N2, N, SN_LESS(N1, N2, N, Y))


#define SN_RECURSIVE_F(a, ...) SN_TOP_ARG_1(a)(SN_POP_ARG_2(a), __VA_ARGS__)
#define SN_RECURSIVE_L(a) SN_TOP_ARG_1(a)(SN_POP_ARG_2(a), SN_POP_ARG_2(a))

#define SN_RECURSIVE_N(N, F, NUL, ...) SN_APPLY_NEST_N(N, SN_RECURSIVE_F, SN_RECURSIVE_L, MACRO_EXPAND, F, NUL, __VA_ARGS__)

// f, 1, 2, 3, 4 -> f(1, f(2, f(3, 4)))
#define SN_APPLY_EXPAND_F(a, ...) SN_TOP_ARG_1(a)(SN_TOP_ARG_2(a), __VA_ARGS__)
#define SN_APPLY_EXPAND_L(a) SN_TOP_ARG_1(a)(SN_POP_ARG_1(a))
#define SN_APPLY_EXPAND_NE(...) SN_TOP_ARG_1(MACRO_EXPAND(__VA_ARGS__)), SN_POP_ARG_2(__VA_ARGS__)
#define SN_APPLY_EXPAND_N(N, F, ...) SN_APPLY_NEST_N(N, SN_APPLY_EXPAND_F, SN_APPLY_EXPAND_L, SN_APPLY_EXPAND_NE, F, __VA_ARGS__)
#define SN_APPLY_EXPAND(F, ...) SN_APPLY_EXPAND_N(SN_GET_ARG_N(SN_POP_ARG_1(__VA_ARGS__)), F, __VA_ARGS__)

#define SN_STRING_WIDEN_IMPL(x) L ## x
#define SN_STRING_WIDEN(x) SN_STRING_WIDEN_IMPL(x)
#define __WFILE__ SN_STRING_WIDEN(__FILE__)

#ifdef __GNUC__
	struct source_location {
	public:
		static constexpr source_location current(
			const char* file = __builtin_FILE(),
			const char* func = __builtin_FUNCTION(),
			int line = __builtin_LINE(),
			int col = 0
		) noexcept {
			source_location loc;
			loc.m_file = file;
			loc.m_func = func;
			loc.m_line = line;
			loc.m_col = col;
		}
		constexpr source_location() noexcept
			: m_file("Unknown"), m_func(m_file), m_line(0), m_col(0) {}
		constexpr uint_least32_t line() const noexcept { return m_line; }
		constexpr uint_least32_t column() const noexcept { return m_col; }
		constexpr const char* file() const noexcept { return m_file; }
		constexpr const char* func() const noexcept { return m_func; }
	private:
		const char* m_file;
		const char* m_func;
		uint_least32_t m_line;
		uint_least32_t m_col;
	};
#elif defined(_MSC_VER)
	struct source_location {
	public:
		static constexpr source_location current(
			const wchar_t* file = __WFILE__,
			const char* func = __func__,
			int line = __LINE__,
			int col = 0
		) noexcept {
			source_location loc;
			loc.m_file = file;
			loc.m_func = func;
			loc.m_line = line;
			loc.m_col = col;
		}
		constexpr source_location() noexcept
			: m_file(L"Unknown"), m_func("Unknown"), m_line(0), m_col(0) {}
		constexpr uint_least32_t line() const noexcept { return m_line; }
		constexpr uint_least32_t column() const noexcept { return m_col; }
		constexpr const wchar_t* file() const noexcept { return m_file; }
		constexpr const char* func() const noexcept { return m_func; }
	private:
		const wchar_t* m_file;
		const char* m_func;
		uint_least32_t m_line;
		uint_least32_t m_col;
	};
#endif

}

#endif