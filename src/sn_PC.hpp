#ifndef SN_PC_H
#define SN_PC_H

#include "sn_CommonHeader.h"
#include "sn_Assist.hpp"

// ref: https://github.com/keean/Parser-Combinators
// TODO: add examples and get hang of this techniques
// TODO: ref the realization of Lily-Cat https://zhuanlan.zhihu.com/p/25411428?
// TODO: ref the Ninputer/VBF CPS-style parsec http://www.cnblogs.com/Ninputer/archive/2011/07/03/2096944.html
namespace sn_PC {
	
	using std::string;
	using std::size_t;

	namespace srange {
		class stream_range {
			std::string m_str;
		public:
			using iterator = std::string::const_iterator;
			const iterator last;
			const iterator first;
			stream_range(const char* name) : m_str(name), first(m_str.cbegin()), last(m_str.cend()) {}
			stream_range(string name) : m_str(name), first(m_str.cbegin()), last(m_str.cend()) {}
		};
	}

	using srange::stream_range;

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

	template <typename P1, typename P2, typename = P1::is_predicate_type, typename = P2::is_predicate_type>
	constexpr is_assist::is_either<P1, P2> const operator||(const P1& p1, const P2& p2) {
		return is_assist::is_either<P1, P2>(p1, p2);
	}

	template <typename P1, typename P2, typename = P1::is_predicate_type, typename = P2::is_predicate_type>
	constexpr is_assist::is_except<P1, P2> const operator-(const P1& p1, const P2& p2) {
		return is_assist::is_except<P1, P2>(p1, p2);
	}


	constexpr is_assist::is_any is_any;
	constexpr is_assist::is_alnum is_alnum;
	constexpr is_assist::is_alpha is_alpha;
	constexpr is_assist::is_blank is_blank;
	constexpr is_assist::is_cntrl is_cntrl;
	constexpr is_assist::is_digit is_digit;
	constexpr is_assist::is_xdigit is_xdigit;
	constexpr is_assist::is_graph is_graph;
	constexpr is_assist::is_lower is_lower;
	constexpr is_assist::is_upper is_upper;
	constexpr is_assist::is_punct is_punct;
	constexpr is_assist::is_space is_space;
	constexpr is_assist::is_char is_eof(EOF);
	
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

	// stream advance if matched and result += symbol
	template <typename P>
	class recogniser_accept {
		const P& m_p;
	public:
using is_parser_type = std::true_type;
using is_handle_type = std::false_type;
using has_side_effects = std::false_type;
using result_type = std::string;
const std::size_t rank;

constexpr explicit recogniser_accept(const P& p)
	: m_p(p), rank(p.rank) {}
template <typename It, typename R, typename Base = default_base>
bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
	int sym;
	if (i == r.last)
		sym = EOF;
	else
		sym = *it;
	if (!m_p(sym))
		return false;
	++i;
	if (result != nullptr)
		result->push_back(sym);
	return true;
}
string enbf(unique_defs& defs = nullptr) const {
	return p.name();
}
	};

	template <typename P, typename = typename P::is_predicate_type>
	constexpr recogniser_accept<P> accept(const P& p) {
		return recogniser_accept<P>(p);
	}

	// String
	class accept_str {
		const char* m_s;
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = std::false_type;
		using result_type = std::string;
		const std::size_t rank = 0;

		constexpr explicit accept_str(const char* s)
			: m_s(s) {}
		template <typename It, typename R, typename Base = default_base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			for (auto j = m_s; *j != '\0'; ++j) {
				if (i == r.last || *i != *j)
					return false;
				++i;
			}
			if (result != nullptr)
				result->append(s);
			return true;
		}
		string ebnf(unique_defs* defs = nullptr) const {
			return "\"" + string(m_s) + "\"";
		}
	};

	class parser_succ {
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = std::false_type;
		using result_type = void;
		const std::size_t rank = 0;

		constexpr explicit parser_succ() {}
		template <typename It, typename R, typename Base = default_base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			return true;
		}
		string ebnf(unique_defs* defs = nullptr) const {
			return "succ";
		}
	};

	class parser_fail {
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = std::false_type;
		using result_type = void;
		const std::size_t rank = 0;

		constexpr explicit parser_fail() {}
		template <typename It, typename R, typename Base = default_base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			return false;
		}
		string ebnf(unique_defs* defs = nullptr) const {
			return "fail";
		}
	};

	// lifting
	template <typename F, typename Base, typename = void>
	class call_any {};
	template <typename F, typename Base>
	class call_any<F, Base, std::enable_if_t<std::is_same<Base, default_base>::value>> {
		const F m_f;
	public:
		explicit call_any(const F& f)
			: m_f(f) {}
		template <typename R, typename Rs, std::size_t ...I>
		void any(R* r, int j, Rs& rs, Base* st, size_t...) {
			f(r, j, std::get<I>(rs)...);
		}
	};
	template <typename F, typename Base>
	class call_any<F, Base, std::enable_if_t<!std::is_same<Base, default_base>::value>> {
		const F m_f;
	public:
		explicit call_any(const F& f)
			: m_f(f) {}
		template <typename R, typename Rs, std::size_t ...I>
		void any(R* r, int j, Rs& rs, Base* st, size_t...) {
			f(r, j, std::get<I>(rs)..., st);
		}
	};

	template <typename F, typename ...Ps>
	class fmap_choice {
		using functor_traits = sn_Assist::sn_function_traits::function_traits<F>;
		using tuple_type = std::tuple<Ps...>;
		using tmp_type = std::tuple<typename Ps::result_type...>;
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = std::true_type;
		using result_type = std::remove_pointer_t<typename functor_traits::template args<0>::type>;
	private:
		const tuple_type ps;
		const F f;

		template <typename It, typename R, typename Base, typename Rs, size_t I0, size_t ...Is>
		size_t any_parsers(It& i, const R& r, Base* st, Rs& rs, size_t, size_t...) const {
			if (std::get<I0>(ps)(i, r, &std::get<I0>(rs), st)) {
				return I0;
			}
			return any_parsers<It, R, Base, Rs, Is...>(i, r, st, rs, Is...);
		}
		template <typename It, typename R, typename Base, typename Rs, size_t I0>
		size_t any_parsers(It& i, const R& r, Base* st, Rs& rs, size_t) const {
			if (std::get<I0>(ps)(i, r, &std::get<I0>(rs), st)) {
				return I0;
			}
			return -1;
		}
		template <typename It, typename R, typename Base, size_t ...I>
		bool fmap_any(It& i, const R& r, size_sequence<I...> seq, result_type* result, Base* st) const {
			tmp_type tmp{};
			const It first = i;
			const size_t j = any_parsers<It, R, Base, tmp_type, I...>(i, r, st, tmp, I...);
			if (j >= 0) {
				if (result != nullptr) {
					call_any<F, Base> call_f(f);
					try {
						call_f.template any<result_type, tmp_type, I...>(result, j, tmp, st, I...);
					}
					catch (std::runtime_error& e) {
						throw parse_error(e.what(), *this, first, i, r);
					}
				}
				return true;
			}
			return false;
		}
	public:
		const int rank = 1;
		constexpr explicit fmap_choice(const F& f, const Ps&... ps)
			: f(f), ps(ps...) {}
		template <typename It, typename R, typename Base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			return fmap_any(i, r, range<0, sizeof...(Ps)>{}, result, st);
		}

		class choice_ebnf {
			const int rank;
			unique_defs* defs;

		public:
			choice_ebnf(int r, unique_defs* d) : rank(r), defs(d) {}
			template <typename P>
			string operator() (const string& s, P&& p) const {
				if (s.length() == 0) {
					return format_name(p, rank, defs);
				}
				return s + " | " + format_name(p, rank, defs);
			}
		};

		string ebnf(unique_defs* defs = nullptr) const {
			return fold_tuple(choice_ebnf(rank, defs), string(), ps);
		}
	};

	template <typename F, typename ...Ps>
	constexpr const fmap_choice<F, Ps...> any(const F& f, const Ps&... ps) {
		return fmap_choice<F, Ps...>(f, ps...);
	}

	
	template <typename F, typename Base, typename = void>
	class call_all {};
	template <typename F, typename Base>
	class call_all<F, Base, std::enable_if_t<std::is_same<Base, default_base>::value>> {
		const F m_f;
	public:
		explicit call_all(const F& f)
			: m_f(f) {}
		template <typename R, typename Rs, std::size_t ...I>
		void all(R* r, Rs& rs, Base* st, size_t...) {
			f(r, std::get<I>(rs)...);
		}
	};
	template <typename F, typename Base>
	class call_all<F, Base, std::enable_if_t<!std::is_same<Base, default_base>::value>> {
		const F m_f;
	public:
		explicit call_all(const F& f)
			: m_f(f) {}
		template <typename R, typename Rs, std::size_t ...I>
		void all(R* r, Rs& rs, Base* st, size_t...) {
			f(r, std::get<I>(rs)..., st);
		}
	};

	template <typename F, typename ...Ps>
	class fmap_sequence {
		using functor_traits = sn_Assist::sn_function_traits::function_traits<F>;
		using tuple_type = std::tuple<Ps...>;
		using tmp_type = std::tuple<typename Ps::result_type...>;
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = std::true_type;
		using result_type = std::remove_pointer_t<typename functor_traits::template args<0>::type>;
	private:
		const tuple_type ps;
		const F f;

		template <typename It, typename R, typename Base, typename Rs, size_t I0, size_t ...Is>
		size_t all_parsers(It& i, const R& r, Base* st, Rs& rs, size_t, size_t...) const {
			if (std::get<I0>(ps)(i, r, &std::get<I0>(rs), st)) {
				return all_parsers<It, R, Base, Rs, Is...>(i, r, st, rs, Is...);
			}
			return false;
		}
		template <typename It, typename R, typename Base, typename Rs, size_t I0>
		size_t all_parsers(It& i, const R& r, Base* st, Rs& rs, size_t) const {
			return std::get<I0>(ps)(i, r, &std::get<I0>(rs), st);
		}
		template <typename It, typename R, typename Base, size_t ...I>
		bool fmap_all(It& i, const R& r, size_sequence<I...> seq, result_type* result, Base* st) const {
			tmp_type tmp{};
			const It first = i;
			if (all_parsers<It, R, Base, tmp_type, I...>(i, r, st, tmp, I...)) {
				if (result != nullptr) {
					call_all<F, Base> call_f(f);
					try {
						call_f.template all<result_type, tmp_type, I...>(result, j, tmp, st, I...);
					}
					catch (std::runtime_error& e) {
						throw parse_error(e.what(), *this, first, i, r);
					}
				}
				return true;
			}
			return false;
		}
	public:
		const int rank = 0;
		constexpr explicit fmap_sequence(const F& f, const Ps&... ps)
			: f(f), ps(ps...) {}
		template <typename It, typename R, typename Base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			return fmap_all(i, r, range<0, sizeof...(Ps)>{}, result, st);
		}

		class sequence_ebnf {
			const int rank;
			unique_defs* defs;

		public:
			sequence_ebnf(int r, unique_defs* d) : rank(r), defs(d) {}
			template <typename P>
			string operator() (const string& s, P&& p) const {
				if (s.length() == 0) {
					return format_name(p, rank, defs);
				}
				return s + " , " + format_name(p, rank, defs);
			}
		};

		string ebnf(unique_defs* defs = nullptr) const {
			return fold_tuple(sequence_ebnf(rank, defs), string(), ps);
		}
	};

	template <typename F, typename ...Ps>
	constexpr const fmap_sequence<F, Ps...> all(const F& f, const Ps&... ps) {
		return fmap_sequence<F, Ps...>(f, ps...);
	}

	template <typename P1, typename P2>
	class combinator_choice {
		const P1 p1;
		const P2 p2;
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = std::integral_constant<bool, P1::has_side_effects::value || P2::has_side_effects::value>;
		using result_type = typename least_general<P1, P2>::result_type;
		const size_t rank = 1;
		
		constexpr combinator_choice(const P1& p1, const P2& p2)
			: p1(p1), p2(p2) {}
		template <typename It, typename R, typename Base = default_base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			const It first = i;
			if (p1(i, r, result, st)) {
				return true;
			}
			if (first != i) {
				throw parse_error("Failed parser consumed input", p1, first, i, r);
			}
			if (p2(i, r, result, st)) {
				return true;
			}
			return false;
		}

		string ebnf(unique_defs* defs = nullptr) const {
			return format_name(p1, rank, defs) + " | " + format_name(p2, rank, defs);
		}
	};

	template <typename P1, typename P2, 
		typename = std::enable_if_t<std::is_same<typename P1::is_parser_type, std::true_type>::value
			|| std::is_same<typename P1::is_handle_type, std::true_type>::value>, 
		typename = std::enable_if_t<std::is_same<typename P2::is_parser_type, std::true_type>::value
			|| std::is_same<typename P2::is_handle_type, std::true_type>::value>,
		typename = std::enable_if_t<is_assist::is_compatible<typename P1::result_type, typename P2::result_type>::value,
			std::pair<typename P1::result_type, typename P2::result_type>>>
	constexpr const combinator_choice<P1, P2> operator||(const P1& p1, const P2& p2) {
		return combinator_choice<P1, P2>(p1, p2);
	}

	template <typename P1, typename P2>
	class combinator_sequence {
		const P1 p1;
		const P2 p2;
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = std::integral_constant<bool, P1::has_side_effects::value || P2::has_side_effects::value>;
		using result_type = typename least_general<P1, P2>::result_type;
		const size_t rank = 0;

		constexpr combinator_sequence(const P1& p1, const P2& p2)
			: p1(p1), p2(p2) {}
		template <typename It, typename R, typename Base = default_base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			return (p1(i, r, result, st)) && (p2(i, r, result, st));
		}

		string ebnf(unique_defs* defs = nullptr) const {
			return format_name(p1, rank, defs) + " , " + format_name(p2, rank, defs);
		}
	};

	template <typename P1, typename P2,
		typename = std::enable_if_t<std::is_same<typename P1::is_parser_type, std::true_type>::value
		|| std::is_same<typename P1::is_handle_type, std::true_type>::value>,
		typename = std::enable_if_t<std::is_same<typename P2::is_parser_type, std::true_type>::value
		|| std::is_same<typename P2::is_handle_type, std::true_type>::value>,
		typename = std::enable_if_t<is_assist::is_compatible<typename P1::result_type, typename P2::result_type>::value,
		std::pair<typename P1::result_type, typename P2::result_type>>>
		constexpr const combinator_sequence<P1, P2> operator&&(const P1& p1, const P2& p2) {
		return combinator_sequence<P1, P2>(p1, p2);
	}

	template <typename P>
	class combinator_many {
		const P p;
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = typename P::has_side_effects;
		using result_type = typename P::result_type;
		const size_t rank = 0;

		constexpr combinator_many(const P& p)
			: p(p) {}
		template <typename It, typename R, typename Base = default_base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			It first = i;
			while (p(i, r, result, st))
				first = i;
			if (first != i) {
				throw parse_error("Failed many parser consumed input", p, first, i, r);
			}
			return true;
		}

		string ebnf(unique_defs* defs = nullptr) const {
			return "{" + p.ebnf(defs) + "}";
		}
	};

	template <typename P, typename = std::enable_if_t<std::is_same<typename P::is_parser_type, std::true_type>::value || std::is_same<typename P::is_handle_type, std::true_type>::value>>
	constexpr const combinator_many<P> many(const P& p) {
		return combinator_many<P>(p);
	}

	template <typename P>
	class combinator_except {
		const P p;
		const char* x;
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = typename P::has_side_effects;
		using result_type = typename P::result_type;
		const size_t rank = 0;

		constexpr combinator_except(const P& p, const char* x)
			: p(p), x(x) {}
		template <typename It, typename R, typename Base = default_base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			result_type tmp;
			if (p(i, r, &tmp, st)) {
				if (x != tmp) {
					if (result != nullptr) {
						*result = tmp;
					}
					return true;
				}
			}
			return false;
		}

		string ebnf(unique_defs* defs = nullptr) const {
			return p.ebnf(defs) + " - \"" + x + "\"";
		}
	};

	template <typename P, typename = std::enable_if_t<std::is_same<typename P::is_parser_type, std::true_type>::value || std::is_same<typename P::is_handle_type, std::true_type>::value>>
	constexpr const combinator_except<P> many(const P& p, const char* x) {
		return combinator_except<P>(p, x);
	}

	template <typename P>
	class combinator_discard {
		const P p;
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = typename P::has_side_effects;
		using result_type = void;
		const size_t rank;

		constexpr combinator_discard(const P& p_)
			: p(p_), rank(p_.rank) {}
		template <typename It, typename R, typename Base = default_base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			typename P::result_type* const discard_result = nullptr;
			return p(i, r, discard_result, st);
		}

		string ebnf(unique_defs* defs = nullptr) const {
			return p.ebnf(defs);
		}
	};

	template <typename P, typename = std::enable_if_t<std::is_same<typename P::is_parser_type, std::true_type>::value || std::is_same<typename P::is_handle_type, std::true_type>::value>>
	constexpr const combinator_discard<P> discard(const P& p) {
		return combinator_discard<P>(p);
	}

	template <typename It, typename R, typename Syn = void, typename Base = default_base>
	class parser_handle {
		struct holder_base {
			virtual ~holder_base() {}
			virtual bool parse(It& i, const R& r, Syn* result = nullptr, Base* st = nullptr) const = 0;
			virtual string ebnf(unique_defs* defs = nullptr) const = 0;
		};

		template <typename P>
		class holder_poly : public holder_base {
			const P p;
		public:
			explicit holder_poly(const P& p_) : p(p_) {}
			explicit holder_poly(P&& p_) : p(std::forward<P>(p_)) {}
			virtual bool parse(It& i, const R& r, Syn* result = nullptr, Base* st = nullptr) const override {
				return p(i, r, result, st);
			}
			virtual string ebnf(unique_defs* defs = nullptr) const override {
				return p.ebnf(defs);
			}
		};

		std::shared_ptr<const holder_base> p;
	public:
		using is_parser_type = std::false_type;
		using is_handle_type = std::true_type;
		using has_side_effects = std::true_type;
		using result_type = Syn;
		const size_t rank = 0;

		constexpr parser_handle() {}
		template <typename P, typename = std::enable_if_t<std::is_same<typename P::is_parser_type, std::true_type>::value>>
		constexpr parser_handle(const P& p_)
			: p(new holder_poly<P>(p_)) {}
		template <typename P, typename = std::enable_if_t<std::is_same<typename P::is_parser_type, std::true_type>::value>>
		constexpr parser_handle(P&& p_)
			: p(new holder_poly<P>(std::forward<P>(p_))) {}
		constexpr parser_handle(const parser_handle& rhs) : p(rhs.p) {}
		constexpr parser_handle(parser_handle&& rhs) : p(std::move(rhs.p)) {}
		template <typename P, typename = typename enable_if<is_same<typename P::is_parser_type, true_type>::value>::type>
		parser_handle& operator= (P const &p_) {
			p = std::shared_ptr<const holder_base>(new holder_poly<P>(p_));
			return *this;
		}
		parser_handle& operator= (const parser_handle& rhs) {
			p = rhs.p;
			return *this;
		}
		parser_handle& operator= (parser_handle&& rhs) {
			p = std::move(rhs.p);
			return *this;
		}
		
		bool operator()(It& i, const R& r, Syn* result = nullptr, Base* st = nullptr) const override {
			assert(p != nullptr);
			return p->parse(i, r, result, st);
		}
		string ebnf(unique_defs* defs = nullptr) const {
			return p->ebnf(defs);
		}
	};

	template <typename P>
	class parser_ref {
		const P* p;
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = typename P::has_side_effects;
		using result_type = typename P::result_type;
		const int rank = 0;
		const char* name;
		constexpr parser_ref(const char* name_, const P* q)
			: p(q), name(name_) {}
		template <typename It, typename R, typename Base = default_base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			return (*p)(i, r, result, st);
		}
		string ebnf(unique_defs* defs = nullptr) const {
			if (defs != nullptr) {
				auto i = defs->find(name);
				if (i == defs->end()) {
					auto i = (defs->emplace(name, name)).first;
					string const n = p->ebnf(defs);
					i->second = n;
				}
			}
			return name;
		}
	};

	template <typename P, typename = typename P::is_parser_type>
	constexpr parser_ref<P> reference(const char* name, const P* q) {
		return parser_ref(name, q);
	}

	template <typename F>
	class parser_fix {
		using parser_type = typename sn_Assist::sn_function_traits::function_traits<F>::result_type;
		const parser_type p;
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = typename parser_type::has_side_effects;
		using result_type = typename parser_type::result_type;
		const int rank = 0;
		const char* name;

		constexpr explicit parser_fix(const char* n, F f)
			: p(f(reference(n, &p))), name(n) {}
		template <typename It, typename R, typename Base = default_base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			return p(i, r, result, st);
		}
		string ebnf(unique_defs* defs = nullptr) const {
			const string n = p.ebnf(defs);
			if (defs != nullptr) {
				defs->emplace(name, n);
			}
			return name;
		}
	};

	template <typename F>
	constexpr parser_fix<F> fix(char const* n, F f) {
		return parser_fix<F>{n, f};
	}

	template <typename P>
	class parser_log {
		const P p;
		const string msg;

	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = typename P::has_side_effects;
		using result_type = typename P::result_type;
		const int rank;

		constexpr explicit parser_log(const string& s, const P& p_)
			: p(p_), msg(s), rank(p_.rank) {}

		template <typename It, typename R, typename Base = default_base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			const It x = i;
			bool const b = p(i, r, result, st);
#ifdef SN_PC_DEBUG
			if (b) {
				std::cout << msg << ": ";
				if (result != nullptr) {
					cout << *result;
				}
				std::cout << " @" << (x - r.first) << " - " << (i - r.first) << "\n";
			}
#endif
			return b;
		}

		string ebnf(unique_defs* defs = nullptr) const {
			return p.ebnf(defs);
		}
	};

	template <typename P, typename = std::enable_if_t<is_same<typename P::is_parser_type, true_type>::value || is_same<typename P::is_handle_type, true_type>::value>>
	constexpr parser_log<P> log(const string& s, const P& p) {
		return parser_log<P>(s, p);
	}

	template <typename P>
	class parser_try {
		const P p;
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = typename P::has_side_effects;
		using result_type = typename P::result_type;
		const size_t rank;

		constexpr parser_try(const P& p_)
			: p(p_), rank(p_.rank) {}
		template <typename It, typename R, typename Base = default_base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			const It first = i;
			if (p(i, r, result, st))
				return true;
			i = first;
			return false;
		}

		string ebnf(unique_defs* defs = nullptr) const {
			return p.ebnf(defs);
		}
	};

	template <typename P>
	class parser_try_side {
		const P p;
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = typename P::has_side_effects;
		using result_type = typename P::result_type;
		const size_t rank;

		constexpr parser_try_side(const P& p_)
			: p(p_), rank(p_.rank) {}
		template <typename It, typename R, typename Base = default_base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			const It first = i;
			Base inh;
			if (st != nullptr)
				inh = *st;
			if (p(i, r, result, st))
				return true;
			i = first;
			if (st != nullptr)
				*st = inh;
			return false;
		}

		string ebnf(unique_defs* defs = nullptr) const {
			return p.ebnf(defs);
		}
	};

	template <typename P, typename = std::enable_if_t<
		(
		std::is_same<typename P::is_parser_type, std::true_type>::value
		|| std::is_same<typename P::is_handle_type, std::true_type>::value
		) && std::is_same<typename P::has_side_effects, std::false_type>::value>
	>
	constexpr parser_try<P> attempt(P const& p) {
		return parser_try<P>(p);
	}

	template <typename P, typename = std::enable_if_t<
		(
		std::is_same<typename P::is_parser_type, std::true_type>::value
		|| std::is_same<typename P::is_handle_type, std::true_type>::value
		) && std::is_same<typename P::has_side_effects, std::true_type>::value>
	>
	constexpr parser_try_side<P> attempt(P const& p) {
		return parser_try_side<P>(p);
	}

	template <typename P>
	class parser_strict {
		const P p;
		const char* err;
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = typename P::has_side_effects;
		using result_type = typename P::result_type;
		const size_t rank;

		constexpr parser_strict(const char* s, const P& p_)
			: p(p_), rank(p_.rank), err(s) {}
		template <typename It, typename R, typename Base = default_base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			const It first = i;
			if (!p(i, r, result, st))
				throw parse_error(err, p, first, r);
			return true;
		}

		string ebnf(unique_defs* defs = nullptr) const {
			return p.ebnf(defs);
		}
	};

	template <typename P, typename = std::enable_if_t<std::is_same<typename P::is_parser_type, std::true_type>::value
		|| std::is_same<typename P::is_handle_type, std::true_type>::value>>
		constexpr parser_strict<P> strict(char const* s, P const& p) {
		return parser_strict<P>(s, p);
	}

	template <typename P, typename Name>
	class parser_name {
		const P p;
		const Name n;
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = typename P::has_side_effects;
		using result_type = typename P::result_type;
		const size_t rank;

		constexpr parser_name(Name&& m, size_t r, const P& p_)
			: p(p_), rank(r), n(std::forward<Name>(m)) {}
		template <typename It, typename R, typename Base = default_base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			return p(i, r, result, st);
		}

		string ebnf(unique_defs* defs = nullptr) const {
			return n(defs);
		}
	};

	template <typename P, typename N, typename = std::enable_if<std::is_same<typename P::is_parser_type, std::true_type>::value
		|| std::is_same<typename P::is_handle_type, std::true_type>::value>>
	constexpr parser_name<P, N> rename(N&& n, int r, P const& p) {
		return parser_name<P, N>(std::forward<N>(n), r, p);
	}

	template <typename P>
	class parser_def {
		const P p;
	public:
		using is_parser_type = std::true_type;
		using is_handle_type = std::false_type;
		using has_side_effects = typename P::has_side_effects;
		using result_type = typename P::result_type;
		const size_t rank;
		const char* name;

		constexpr parser_def(const char* n, const P& p_)
			: p(p_), rank(p_.rank), name(n) {}
		template <typename It, typename R, typename Base = default_base>
		bool operator()(It& i, const R& r, string* result = nullptr, Base* st = nullptr) const {
			return p(i, r, result, st);
		}

		string ebnf(unique_defs* defs = nullptr) const {
			const string n = p.ebnf(defs);
			if (defs != nullptr)
				defs->emplace(name, n);
			return name;
		}
	};


	template <typename P, typename = std::enable_if_t<std::is_same<typename P::is_parser_type, std::true_type>::value
		|| std::is_same<typename P::is_handle_type, std::true_type>::value>>
	constexpr parser_def<P> define(char const* s, P const& p) {
		return parser_def<P>(s, p);
	}

	template <typename P>
	struct option_name {
		const P p;
		constexpr option_name(const P& p_) : p(p_) {}
		string operator() (unique_defs* defs = nullptr) const {
			return "[" + p.ebnf(defs) + "]";
		}
	};

	constexpr parser_succ succ;
	constexpr parser_fail fail;

	template <typename P> constexpr auto option(P const& p)
		-> decltype(rename(option_name<P>(p), 0, p || succ)) {
		return rename(option_name<P>(p), 0, p || succ);
	}
	
	template <typename P> 
	struct some_name {
		const P p;
		constexpr some_name(const P& q) : p(q) {}
		string operator() (unique_defs* defs = nullptr) const {
			return "{" + p.ebnf(defs) + "}+";
		}
	};

	template <typename P> 
	constexpr auto some(P const& p)
		-> decltype(rename(some_name<P>(p), 0, p && many(p))) {
		return rename(some_name<P>(p), 0, p && many(p));
	}

	template <typename P, typename Q> 
	struct sepby_name {
		const P p;
		const Q q;
		constexpr sepby_name(const P& p, const Q& q) : p(p), q(q) {}
		string operator() (unique_defs* defs = nullptr) const {
			return p.ebnf(defs) + ", {" + q.ebnf(defs) + ", " + p.ebnf() + "}";
		}
	};

	template <typename P, typename Q> constexpr auto sep_by(P const& p, Q const& q)
		-> decltype (rename(sepby_name<P, Q>(p, q), 0, p && many(discard(q) && p))) {
		return rename(sepby_name<P, Q>(p, q), 0, p && many(discard(q) && p));
	}

	
	constexpr auto first_token = discard(many(accept(is_space)));

	template <typename P> 
	struct tok_name {
		const P p;
		constexpr tok_name(const P& q) : p(q) {}
		string operator() (unique_defs* defs = nullptr) const {
			return p.ebnf(defs);
		}
	};

	template <typename R> 
	constexpr auto tokenise(const R& r)
		-> decltype(rename(tok_name<R>(r), 0, r && first_token)) {
		return rename(tok_name<R>(r), 0, r && first_token);
	}

}






#endif