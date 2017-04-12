#ifndef SN_PC_H
#define SN_PC_H

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"

// ref: https://github.com/keean/Parser-Combinators
namespace sn_PC {
	
	using std::string;
	using std::size_t;

	namespace is_assist {

		using std::true_type;
		using std::false_type;
		

#define SN_PC_IS_ASSIST(NAME, NAME2, RANK, STR) \
	struct NAME { \
		using is_predicate_type = true_type; \
		static constexpr size_t rank = RANK; \
		constexpr NAME() {} \
		bool operator()(const int c) const { \
			return NAME2(c) != 0; \
		} \
		string name() const { \
			return STR; \
		} \
	}; \

		struct is_any {
			using is_predicate_type = true_type;
			static constexpr size_t rank = 0;
			constexpr is_any() {}
			bool operator() (const int c) const {
				return c != EOF;
			}
			string name() const {
				return "epsilon";
			}
		};

		struct is_alnum {
			using is_predicate_type = true_type;
			static constexpr size_t rank = 0;
			constexpr is_alnum() {}
			bool operator() (const int c) const {
				return ::isalnum(c) != 0;
			}
			string name() const {
				return "alphanumeric";
			}
		};

		struct is_alpha {
			using is_predicate_type = true_type;
			static constexpr size_t rank = 0;
			constexpr is_alpha() {}
			bool operator() (const int c) const {
				return ::isalpha(c) != 0;
			}
			string name() const {
				return "alphabetic";
			}
		};

		struct is_blank {
			using is_predicate_type = true_type;
			static constexpr size_t rank = 0;
			constexpr is_blank() {}
			bool operator() (const int c) const {
				return ::isblank(c) != 0;
			}
			string name() const {
				return "empty";
			}
		};

		struct is_cntrl {
			using is_predicate_type = true_type;
			static constexpr size_t rank = 0;
			constexpr is_cntrl() {}
			bool operator() (const int c) const {
				return ::iscntrl(c) != 0;
			}
			string name() const {
				return "control";
			}
		};

		struct is_digit {
			using is_predicate_type = true_type;
			static constexpr size_t rank = 0;
			constexpr is_digit() {}
			bool operator() (const int c) const {
				return ::isdigit(c) != 0;
			}
			string name() const {
				return "digit";
			}
		};

		struct is_xdigit {
			using is_predicate_type = true_type;
			static constexpr size_t rank = 0;
			constexpr is_xdigit() {}
			bool operator() (const int c) const {
				return ::isxdigit(c) != 0;
			}
			string name() const {
				return "hexdigit";
			}
		};

		struct is_graph {
			using is_predicate_type = true_type;
			static constexpr size_t rank = 0;
			constexpr is_graph() {}
			bool operator() (const int c) const {
				return ::isgraph(c) != 0;
			}
			string name() const {
				return "graphic";
			}
		};

		struct is_lower {
			using is_predicate_type = true_type;
			static constexpr size_t rank = 0;
			constexpr is_lower() {}
			bool operator() (const int c) const {
				return ::islower(c) != 0;
			}
			string name() const {
				return "lowercase";
			}
		};

		struct is_upper {
			using is_predicate_type = true_type;
			static constexpr size_t rank = 0;
			constexpr is_upper() {}
			bool operator() (const int c) const {
				return ::isupper(c) != 0;
			}
			string name() const {
				return "uppercase";
			}
		};

		struct is_punct {
			using is_predicate_type = true_type;
			static constexpr size_t rank = 0;
			constexpr is_punct() {}
			bool operator() (const int c) const {
				return ::ispunct(c) != 0;
			}
			string name() const {
				return "punctuation";
			}
		};

		struct is_space {
			using is_predicate_type = true_type;
			static constexpr size_t rank = 0;
			constexpr is_space() {}
			bool operator() (const int c) const {
				return ::isspace(c) != 0;
			}
			string name() const {
				return "space";
			}
		};

		struct is_eol {
			using is_predicate_type = true_type;
			static constexpr size_t rank = 0;
			constexpr is_eol() {}
			bool operator() (const int c) const {
				return c == '\n';
			}
			string name() const {
				return "EOL";
			}
		};

		struct is_char {
			const char k;
			using is_predicate_type = true_type;
			static constexpr size_t rank = 0;
			constexpr is_char(const char c): k(c) {}
			bool operator() (const int c) const {
				return k == c;
			}
			string name() const {
				return "'" + string(1, k) + "'";
			}
		};

		constexpr is_char is_eof(EOF);

		template <typename P, typename = P::is_predicate_type>
		string format_name(const P& p, const int rank) {
			if (p.rank > rank)
				return "(" + p.name() + ")";
			else
				return p.name();
		}
		
		template <typename P1, typename P2>
		struct is_either {
			const P1 p1;
			const P2 p2;
			using is_predicate_type = true_type;
			static constexpr size_t rank = 1;
			constexpr is_either(const P1& p1_, const P2& p2_)
				: p1(p1_), p2(p2_) {}
			bool operator() (const int c) const {
				return p1(c) || p2(c);
			}
			string name() const{
				return p1.name() + " | " + p2.name();
			}
		};

		template <typename P1, typename P2, typename = P1::is_predicate_type, typename = P2::is_predicate_type>
		constexpr is_either<P1, P2> const operator||(const P1& p1, const P2& p2) {
			return is_either<P1, P2>(p1, p2);
		}

		template <typename P1, typename P2>
		struct is_except {
			const P1 p1;
			const P2 p2;
			using is_predicate_type = true_type;
			static constexpr size_t rank = 0;
			constexpr is_except(const P1& p1_, const P2& p2_)
				: p1(p1_), p2(p2_) {}
			bool operator() (const int c) const {
				return p1(c) && !p2(c);
			}
			string name() const {
				return format_name(p1, rank) + " - " + format_name(p2, rank);
			}
		};

		template <typename P1, typename P2, typename = P1::is_predicate_type, typename = P2::is_predicate_type>
		constexpr is_except<P1, P2> const operator-(const P1& p1, const P2& p2) {
			return is_except<P1, P2>(p1, p2);
		}

		template <typename A, typename B>
		struct is_compat {
			using PA = std::add_pointer_t<A>;
			using PB = std::add_pointer_t<B>;
			static constexpr bool value = std::is_convertible<PA, PB>::value;
		};

		template <typename A, typename B>
		struct is_compatible {
			static constexpr bool value = is_compat<A, B>::value || is_compat<B, A>::value;
		};

	}

	struct default_base {};

	using unique_defs = std::map<std::string, std::string>;

	struct parse_error : public std::runtime_error {
		template <typename P, typename It, typename R>
		static std::string message(const string& what, const P& p, const It& f, const It& l, const R& r) {
			std::stringstream err;
			It i(r.first);
			It line_start(r.first);
			std::size_t row = 1;
			while ((i != r.last) && (i != f)) {
				if (*i == '\n') {
					++row;
					line_start = ++i;
				}
				else
					++i;
			}
			err << what << " at line: " << row << " column: " << f - line_start + 1 << endl;

			bool in = true;
			for (It i(line_start); (i != r.last) && (in || *i != '\n'); ++i) {
				if (i == 1) {
					in = false;
				}
				is_assist::is_space s;
				if (s(*i)) {
					err << ' ';
				}
				else {
					err << static_cast<char>(*i);
				}
			}
			err << '\n';

			i = line_start;
			while (i != f) {
				err << ' ';
				++i;
			}
			err << '^';
			++i;

			if (i != l) {
				++i;
				while (i != l) {
					err << '-';
					++i;
				}
				err << '^';
			}
			err << '\n' << "expecting: ";

			unique_defs defs;
			err << p.ebnf(&defs) << '\n' << "where:" << '\n';
			for (const auto& d : defs) {
				err << '\t' << d.first << " = " << d.second << ";\n";
			}
			return err.str();
		}

		template <typename P, typename It, typename R>
		parse_error(const string& what, const P& p, const It& f, const It& l, const R& r)
			: std::runtime_error(message(what, p, f, l, r)) {}

	};

	template <size_t ...Is>
	struct size_sequence {};
	template <size_t F, size_t L, size_t ...Is>
	struct range : range<F, L - 1, L - 1, Is...> {};
	template <size_t F, size_t ...Is>
	struct range<F, F, Is...> : size_sequence<Is...> {};

	template <typename F, typename A, typename T, size_t I0, size_t ...Is>
	A fold_tuple2(F f, A a, T&& t, size_t, size_t...) {
		return fold_tuple2<F, A, T, Is...>(f, f(a, std::get<I0>(t)), t, Is...);
	}
	template <typename F, typename A, typename T, size_t I0>
	A fold_tuple2(F f, A a, T&& t, size_t, size_t...) {
		return f(a, std::get<I0>(t));
	}
	template <typename F, typename A, typename T, size_t ...Is>
	A fold_tuple1(F f, A a, T&& t, size_sequence<Is...>) {
		return fold_tuple2<F, A, T, Is...>(f, a, t, Is...);
	}
	template <typename F, typename A, typename ...Ts>
	A fold_tuple(F f, A a, const std::tuple<Ts...>& t) {
		return fold_tuple1(f, a, t, range<0, sizeof...(Ts)>{});
	}

	template <typename P1, typename P2, typename = void>
	struct least_general {};

	template <typename P1, typename P2>
	struct least_general<P1, P2, 
		std::enable_if_t<is_assist::is_compat<typename P2::result_type, typename P1::result_type>::value
		&& !std::is_same<typename P1::result_type, typename P2::result_type>::value>> {
		using result_type = typename P2::result_type;
	};

	template <typename P1, typename P2>
	struct least_general<P1, P2,
		std::enable_if_t<is_assist::is_compat<typename P1::result_type, typename P2::result_type>::value
		|| std::is_same<typename P1::result_type, typename P2::result_type>::value>> {
		using result_type = typename P1::result_type;
	};

	string concat(const string& sep, const string& str) {
		return str;
	}

	template <typename ...Args>
	string concat(const string& sep, const string& str, const Args& ...strs) {
		string s = str;
		std::initializer_list<int>{ (s += seq + strs, 0)... };
		return s;
	}

	template <typename P, typename = typename P::is_parser_type>
	string format_name(const P& p, const int rank, unique_defs* defs = nullptr) {
		if (p.rank > rank) {
			return "(" + p.ebnf(defs) + ")";
		}
		else
			return p.ebnf(defs);
	}

}






#endif