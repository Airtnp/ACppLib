#ifndef SN_REGEX_D_H
#define SN_REGEX_D_H

#include "sn_CommonHeader.h"

// ref : Cheukyin/CodeSnippet/blob/master/PL/Regex
// Brzozowski's derivative
// TODO: add DFA
namespace sn_RegexD {
	namespace AST {
		// Instance ---- this ----> Exp ---- *Exp ---> Instance
		struct Visitor;
		using VisitorPtr = std::shared_ptr<Visitor>;

		struct Regex {
			std::size_t m_maxGroup;
			virtual void accept(const VisitorPtr& vptr) = 0;
		};

		using RegexPtr = std::shared_ptr<Regex>;

		struct Empty : public Regex, public std::enable_shared_from_this<Empty> {
			void accept(const VisitorPtr& vptr) override;
		};
		struct Null : public Regex, public std::enable_shared_from_this<Null> {
			void accept(const VisitorPtr& vptr) override;
		};
		struct Char : public Regex, public std::enable_shared_from_this<Char> {
			char m_ch;
			Char(char ch) : m_ch(ch) {}
			void accept(const VisitorPtr& vptr) override;
		};
		struct Alt : public Regex, public std::enable_shared_from_this<Alt> {
			RegexPtr m_expl, m_expr;
			Alt(const RegexPtr& l, const RegexPtr& r) : m_expl(l), m_expr(r) {}
			void accept(const VisitorPtr& vptr) override;
		};
		struct Seq : public Regex, public std::enable_shared_from_this<Seq> {
			RegexPtr m_expl, m_expr;
			Seq(const RegexPtr& l, const RegexPtr& r) : m_expl(l), m_expr(r) {}
			void accept(const VisitorPtr& vptr) override;
		};
		struct Rep : public Regex, public std::enable_shared_from_this<Rep> {
			RegexPtr m_exp;
			Rep(const RegexPtr& re) : m_exp(re) {}
			void accept(const VisitorPtr& vptr) override;
		};
		struct Group : public Regex, public std::enable_shared_from_this<Group> {
			RegexPtr m_exp;
			std::size_t m_group;
			Group(const RegexPtr& re, std::size_t g) : m_exp(re), m_group(g) {}
			void accept(const VisitorPtr& vptr) override;
		};
		using EmptyPtr = std::shared_ptr<Empty>;
		using NullPtr = std::shared_ptr<Null>;
		using CharPtr = std::shared_ptr<Char>;
		using AltPtr = std::shared_ptr<Alt>;
		using SeqPtr = std::shared_ptr<Seq>;
		using RepPtr = std::shared_ptr<Rep>;
		using GroupPtr = std::shared_ptr<Group>;
	}

	namespace AST {
		using VisitorPtr = std::shared_ptr<Visitor>;

		struct Visitor
		{
			virtual void visit(const RegexPtr& re) = 0;
			virtual void visit(const EmptyPtr& re) = 0;
			virtual void visit(const NullPtr&  re) = 0;
			virtual void visit(const CharPtr&  re) = 0;
			virtual void visit(const AltPtr&   re) = 0;
			virtual void visit(const SeqPtr&   re) = 0;
			virtual void visit(const RepPtr&   re) = 0;
			virtual void visit(const GroupPtr& re) = 0;
		};
	}

	namespace AST {
		void Empty::accept(const VisitorPtr& vptr) {
			vptr->visit(shared_from_this());
		}

		void Null::accept(const VisitorPtr& vptr) {
			vptr->visit(shared_from_this());
		}

		void Char::accept(const VisitorPtr& vptr) {
			vptr->visit(shared_from_this());
		}

		void Alt::accept(const VisitorPtr& vptr) {
			vptr->visit(shared_from_this());
		}

		void Seq::accept(const VisitorPtr& vptr) {
			vptr->visit(shared_from_this());
		}

		void Rep::accept(const VisitorPtr& vptr) {
			vptr->visit(shared_from_this());
		}

		void Group::accept(const VisitorPtr& vptr) {
			vptr->visit(shared_from_this());
		}
	}

	namespace ASTDump {
		using namespace AST;
		struct ASTDump : public Visitor, public std::enable_shared_from_this<ASTDump> {
			std::string m_tree;
			std::string dump(const RegexPtr& re) {
				m_tree = "";
				visit(re);
				return m_tree;
			}
			void visit(const RegexPtr& re) override {
				re->accept(shared_from_this());
			}
			void visit(const EmptyPtr& re) override {
				m_tree += "(Empty)";
			}
			void visit(const NullPtr& re) override {
				m_tree += "(Null)";
			}
			void visit(const CharPtr& re) override {
				m_tree += ("(Char" + std::string(1, re->m_ch) + ")");
			}
			void visit(const AltPtr& re) override {
				m_tree += "(Alt";
				re->m_expl->accept(shared_from_this());
				m_tree += " ";
				re->m_expr->accept(shared_from_this());
				m_tree += ")";
			}
			void visit(const SeqPtr& re) override {
				m_tree += "(Alt";
				re->m_expl->accept(shared_from_this());
				m_tree += " ";
				re->m_expr->accept(shared_from_this());
				m_tree += ")";
			}
			void visit(const RepPtr& re) override {
				m_tree += "(Rep";
				re->m_exp->accept(shared_from_this());
				m_tree += ")";
			}
			void visit(const GroupPtr& re) override {
				m_tree += "(Group";
				re->m_exp->accept(shared_from_this());
				m_tree += ")";
			}
		};

		using ASTDumpPtr = std::shared_ptr<ASTDump>;

	}

	namespace Cache {
		template <typename T>
		class CachedObject {
		public:
			CachedObject() {
				if (!m_hasRegDelPool) {
					m_hasRegDelPool = true;
					atexit(deletePool);
				}
			}

			void* operator new(std::size_t sz) {
				if (m_freeQueue.empty())
					allocChunk();
				observer_ptr<T> obj = m_freeQueue.front();
				m_freeQueue.pop();
				return obj;
			}

			void operator delete(void* p, std::size_t sz) {
				if (!p)
					return;
				m_freeQueue.push(static_cast<observer_ptr<T>>(p));
			}

			static void deletePool() {
				for (observerptr<T> obj : m_pool)
					::operator delete(obj);
			}

		private:
			static bool m_hasRegDelPool;
			static std::vector<observer_ptr<T>> m_pool;
			static std::size_t m_chunkSize;
			static std::queue<observer_ptr<T>> m_freeQueue;

			static void allocChunk() {
				for (std::size_t i = 0; i < m_chunkSize; ++i) {
					observer_ptr<T> p = static_cast<observer_ptr<T>>(::operator new(sizeof(T)));
					m_pool.push_back(p);
					m_freeQueue.push(p);
				}
			}
		};

		template <typename T>
		bool CachedObject<T>::m_hasRegDelPool = false;

		template <typename T>
		std::vector<observer_ptr<T>> CachedObject<T>::m_pool;

		template <typename T>
		std::size_t CachedObject<T>::m_chunkSize = 20;

		template <typename T>
		std::queue<observer_ptr<T>> CachedObject<T>::m_freeQueue;
	}

	// Brzozowski's derivative
	namespace DERIV {
		using namespace AST;
		// NullCheck(<Empty>) = False
		// NullCheck(<Null>) = True
		// NullCheck(<Char c>) = False
		// NullCheck(<Alt L R>) = NullCheck(L) | NullCheck(R)
		// NullCheck(<Seq F L>) = NullCheck(F) & NullCheck(L)
		// NullCheck(<Rep L>) = True
		struct NullCheck : public Visitor, public std::enable_shared_from_this<NullCheck> {
			bool m_isNull;
			bool check(const RegexPtr& re) {
				visit(re);
				return m_isNull;
			}
			void visit(const RegexPtr& re) override {
				re->accept(shared_from_this());
			}
			void visit(const EmptyPtr& re) override {
				m_isNull = false;
			}
			void visit(const NullPtr& re) override {
				m_isNull = true;
			}
			void visit(const CharPtr& re) override {
				m_isNull = false;
			}
			void visit(const AltPtr& re) override {
				re->m_expl->accept(shared_from_this());
				bool leftNull = m_isNull;
				if (m_isNull)
					return;
				re->m_expr->accept(shared_from_this());
			}
			void visit(const SeqPtr& re) override {
				re->m_expl->accept(shared_from_this());
				if (!m_isNull)
					return;
				re->m_expr->accept(shared_from_this());
			}
			void visit(const RepPtr& re) override {
				m_isNull = true;
			}
			void visit(const GroupPtr& re) override {
				re->m_exp->accept(shared_from_this());
			}
		};

		using NullCheckPtr = std::shared_ptr<NullCheck>;

		// Alt(+) | Seq(*) | Rep(^) | Empty(0) | Null(1) | Char(x)
		// ref: https://en.wikipedia.org/wiki/Brzozowski_derivative
		// D(Empty, c) = Empty
		// D(Null, c) = Empty
		// D(c, c) = Null
		// D('.', c) = Null
		// D(c', c) = Empty, if c' != c
		// D(<Alt L R>, c) = <Alt D(L, c) D(R, c)>
		// D(<Seq F L>, c) = if NullCheck(F). <Alt <Seq D(F, c) L> D(L, c)>
		//                   else.              <Seq D(F, c) L>
		// D(<Rep L>) = < Seq D(L, c) <Rep L> >
		struct Derivative : public Visitor, public std::enable_shared_from_this<Derivative> {
			RegexPtr m_exp;
			char m_ch;
			NullCheckPtr m_isNull;
			RegexPtr drv(const RegexPtr& re, char ch) {
				m_isNull = NullCheckPtr(new NullCheck);
				m_exp = re;
				m_ch = ch;
				visit(re);
				return m_exp;
			}
			void visit(const RegexPtr& re) override {
				re->accept(shared_from_this());
			}
			void visit(const EmptyPtr& re) override {
				m_exp = EmptyPtr(new Empty);
			}
			void visit(const NullPtr& re) override {
				m_exp = EmptyPtr(new Empty);
			}
			void visit(const CharPtr& re) override {
				if (re->m_ch == m_ch)
					m_exp = NullPtr(new Null);
				else if (re->m_ch == '.')
					m_exp = NullPtr(new Null);
				else
					m_exp = EmptyPtr(new Empty);
			}
			void visit(const AltPtr& re) override {
				re->m_expl->accept(shared_from_this());
				RegexPtr expl = m_exp;
				re->m_expr->accept(shared_from_this());
				m_exp = AltPtr(new Alt(expl, m_exp));
			}
			// D(FxL) -> F'xL + L' | F'xL (F neq (Null | Regex) | Regex* | )
			void visit(const SeqPtr& re) override {
				re->m_expl->accept(shared_from_this());
				m_exp = SeqPtr(new Seq(m_exp, re->m_expr));
				RegexPtr expl = m_exp;
				if (m_isNull->check(re->m_expl)) {
					re->m_expr->accept(shared_from_this());
					m_exp = AltPtr(new Alt(expl, m_exp));
				}
			}
			void visit(const RepPtr& re) override {
				re->m_exp->accept(shared_from_this());
				m_exp = SeqPtr(new Seq(m_exp, re));
			}
			void visit(const GroupPtr& re) override {
				re->m_exp->accept(shared_from_this());
			}
		};

		using DerivativePtr = std::shared_ptr<Derivative>;

	}

	namespace Parse {
		using namespace AST;
		
		class ParserException : std::exception {};

		// <Regex>   ::=  <Seq> ( '|' <Regex> )?
		//                ""
		// <Seq>     ::=  <Factor> <Seq>?
		// <Factor>  ::=  ( <CapturedRe> | <CharSet> | <Char> ) <Op>*

		// <CapturedRe> ::=  '(' '?:'? <Regex> ')'
		// <CharSet> ::=  '[' <Char>'+' ']'
		// <Char>    ::=  <any single character>
		// <Op>      ::=  '*' | '+' | '?' | <Range>
		// <Range>   ::=  '{' <Num> ( ',' <Num> )? '}'
		// <Num>     ::=  [0-9]+
		class Parser {
		public:
			RegexPtr operator()(const std::string& str) {
				if (!str.size()) {
					RegexPtr re(NullPtr(new Null));
					re->m_maxGroup = 1;
					return re;
				}

				m_iter = str.begin();
				m_group = 0;
				RegexPtr re = regex();
				re->m_maxGroup = m_group + 1;
				return re;
			}

		private:
			std::string::const_iterator m_iter;
			std::size_t m_group;
			
			RegexPtr regex() {
				if (!*m_iter || *m_iter == '|')
					throw ParserException();
				RegexPtr lre = seq();

				if (!*m_iter || *m_iter == ')')
					return lre;
				if (*m_iter != '|')
					throw ParserException();
				++m_iter;
				
				if (!*m_iter || *m_iter == '|')
					throw ParserException();
				RegexPtr rre = regex();

				return AltPtr(new Alt(lre, rre));
			}

			RegexPtr seq() {
				RegexPtr first = factor();
				if (!*m_iter || *m_iter == '|' || *m_iter == ')')
					return first;
				RegexPtr last = seq();
				return SeqPtr(new Seq(first, last));
			}

			RegexPtr factor() {
				RegexPtr re;
				if (*m_iter == '(')
					re = capturedRe();
				else if (*m_iter == '[')
					re = charSet();
				else if (*m_iter != '*' && *m_iter != '+' && *m_iter != '?' && *m_iter != '{') {
					re = CharPtr(new Char(*m_iter));
					++m_iter;
				}
				else
					throw ParserException();

				while (1) {
					if (*m_iter == '*') {
						re = RepPtr(new Rep(re));
						++m_iter;
					}
					else if (*m_iter == '+') {
						RegexPtr r(RepPtr(new Rep(re)));
						re = SeqPtr(new Seq(re, r));
						++m_iter;
					}
					else if (*m_iter == '?') {
						RegexPtr r(NullPtr(new Null));
						re = AltPtr(new Alt(r, re));
						++m_iter;
					}
					else if (*m_iter == '{') {
						std::pair<int, int> r = range();
						int r1 = r.first < r.second ? r.first : r.second;
						int r2 = r.first < r.second ? r.second : r.first;

						auto helper = [](std::size_t cnt, RegexPtr& re) -> RegexPtr {
							RegexPtr tmp(NullPtr(new Null));
							for (std::size_t i = 0; i < cnt; ++i)
								tmp = SeqPtr(new Seq(re, tmp));
							return tmp;
						};

						RegexPtr tmp(helper(r2, re));
						for (int i = r2 - 1; i >= r1; --i)
							tmp = AltPtr(new Alt(helper(i, re), tmp));
						re = tmp;
					}
					else
						break;
				}

				assert(*m_iter != '*' && *m_iter != '+' && *m_iter != '?' && *m_iter != '{');
				return re;
			}

			std::pair<int, int> range() {
				assert(*m_iter == '{');
				++m_iter;
				skip();
				checkException();
				int n1 = num();
				if (*m_iter == '}') {
					++m_iter;
					return std::pair<int, int>{n1, n1};
				}

				assert(*m_iter == ',');
				++m_iter;
				skip();
				checkException();
				int n2 = num();
				checkException('}');
				++m_iter;

				return std::pair<int, int>{n1, n2};
			}

			RegexPtr capturedRe() {
				checkException('(');
				++m_iter;
				RegexPtr re;
				if (*m_iter == '?' && *(m_iter + 1) == ':') {
					m_iter += 2;
					re = regex();
				}
				else
					re = GroupPtr(new Group(regex(), ++m_group));
				checkException(')');
				++m_iter;
				return re;
			}

			RegexPtr charSet() {
				checkException('[');
				++m_iter;

				std::stack<char> charStack;
				while (*m_iter && *m_iter != ']') {
					charStack.push(*m_iter);
					++m_iter;
				}
				checkException();
				++m_iter;

				if (charStack.empty())
					throw ParserException();

				char ch = charStack.top();
				charStack.pop();
				RegexPtr re(CharPtr(new Char(ch)));

				RegexPtr l;
				while (!charStack.empty()) {
					char ch = charStack.top();
					charStack.pop();
					l = CharPtr(new Char(ch));
					re = AltPtr(new Alt(l, re));
				}

				return re;
			}

			int num() {
				assert(*m_iter >= '0' && *m_iter <= '9');
				int res = 0;
				while (*m_iter >= '0' && *m_iter <= '9') {
					res *= 10;
					res += (*m_iter - '0');
					++m_iter;
				}
				skip();
				checkException2(',', '}');
			}

			void skip() {
				while (*m_iter && (*m_iter == ' ' || *m_iter == '\t'))
					++m_iter;
			}

			void checkException(char ch = '\0') {
				if (ch == '\0') {
					if (!*m_iter)
						throw ParserException();
					else
						return;
				}
				if (*m_iter != ch)
					throw ParserException();
			}
			void checkException2(char ch1 = '\0', char ch2 = '\0') {
				if (ch1 == '\0') {
					if (!*m_iter)
						throw ParserException();
					else
						return;
				}
				if (*m_iter != ch1 && *m_iter != ch2)
					throw ParserException();
			}
		};
	}

	namespace DERIVEngine {
		using namespace AST;
		using DERIV::Derivative;
		using DERIV::NullCheck;
		using DERIV::DerivativePtr;
		using DERIV::NullCheckPtr;
		using Parse::Parser;

		class DerivativeEngine {
		public:
			using str_size_t = std::string::size_type;
			DerivativeEngine(const std::string& str)
				: m_rePtr(m_parse(str)), m_deriv(new Derivative), m_isNull(new NullCheck) {}

			std::string search(const std::string& str) {
				for (str_size_t i = 0; i < str.size(); ++i) {
					str_size_t tail = searchHelper(m_rePtr, str, i);
					if (tail > i)
						return std::string(str.begin() + 1, str.begin() + tail);
				}
			}

			bool match(const std::string& str) {
				return matchHelper(m_rePtr, str, 0);
			}

		private:
			Parser m_parse;
			RegexPtr m_rePtr;
			DerivativePtr m_deriv;
			NullCheckPtr m_isNull;

			str_size_t searchHelper(RegexPtr re, const std::string& str, str_size_t id) {
				str_size_t result = m_isNull->check(re) ? id : 0;
				if (id >= str.size())
					return result;
				str_size_t tmp = searchHelper(m_deriv->drv(re, str[id]), str, id + 1);
				return result < tmp ? tmp : result;
			}
			bool matchHelper(RegexPtr re, const std::string& str, str_size_t id) {
				if (id >= str.size())
					return m_isNull->check(re);
				return matchHelper(m_deriv->drv(re, str[id]), str, id + 1);
			}
		};

	}

	namespace NFA {
		using namespace AST;
		using namespace Parse;
		using namespace Cache;
		struct NFA;
		using NFAPtr = std::shared_ptr<NFA>;

		class NFAConstructor;
		using NFAConstructorPtr = std::shared_ptr<NFAConstructor>;

		struct NFANode : public CachedObject<NFANode> {
			std::map<char, std::vector<NFANode*>> m_edges;
			std::size_t m_group;

			NFANode() : m_group(-1) {}
			NFANode(std::size_t g) : m_group(g) {}

			void addEdge(char c, NFANode* n) {
				auto iter = m_edges.find(c);
				if (iter != m_edges.end())
					iter->second.push_back(n);
				else
					m_edges[c] = std::vector<NFANode*>(1, n);
			}
		};

		struct NFA {
			NFANode* m_start;
			NFANode* m_end;
			std::size_t m_maxGroup;  // () () ()
			std::vector<NFANode*> m_nodes;

			~NFA() {
				for (NFANode* node : m_nodes)
					delete node;
			}
		};

		class NFAConstructor : public Visitor, public std::enable_shared_from_this<NFAConstructor> {
		private:
			NFAPtr m_nfa;
		public:
			NFAPtr construct(const RegexPtr& re) {
				m_nfa.reset(new NFA);
				visit(GroupPtr(new Group(re, 0)));
				m_nfa->m_maxGroup = re->m_maxGroup;
				return m_nfa;
			}

			void buildSimpleNFA(char ch) {
				m_nfa->m_start = new NFANode;
				m_nfa->m_end = new NFANode;
				m_nfa->m_start->addEdge(ch, m_nfa->m_end);

				m_nfa->m_nodes.push_back(m_nfa->m_start);
				m_nfa->m_nodes.push_back(m_nfa->m_end);
			}

			void visit(const RegexPtr& re) override {
				re->accept(shared_from_this());
			}

			void visit(const EmptyPtr& re) override {
				return;
			}

			void visit(const NullPtr& re) override {
				m_nfa->m_start = new NFANode;
				m_nfa->m_end = m_nfa->m_start;
				m_nfa->m_nodes.push_back(m_nfa->m_start);
			}

			void visit(const CharPtr& re) override {
				buildSimpleNFA(re->m_ch);
			}

			void visit(const AltPtr& re) override {
				re->m_expl->accept(shared_from_this());
				NFANode* lstart = m_nfa->m_start;
				NFANode* lend = m_nfa->m_end;
				re->m_expr->accept(shared_from_this());
				NFANode* rstart = m_nfa->m_start;
				NFANode* rend = m_nfa->m_end;

				m_nfa->m_start = new NFANode;
				m_nfa->m_end = new NFANode;
				m_nfa->m_nodes.push_back(m_nfa->m_start);
				m_nfa->m_nodes.push_back(m_nfa->m_end);

				m_nfa->m_start->addEdge(0, lstart);
				m_nfa->m_start->addEdge(0, rstart);

				lend->addEdge(0, m_nfa->m_end);
				rend->addEdge(0, m_nfa->m_end);
			}

			void visit(const SeqPtr& re) override {
				re->m_expl->accept(shared_from_this());
				NFANode* lstart = m_nfa->m_start;
				NFANode* lend = m_nfa->m_end;
				re->m_expr->accept(shared_from_this());
				NFANode* rstart = m_nfa->m_start;
				NFANode* rend = m_nfa->m_end;

				lend->addEdge(0, rstart);

				m_nfa->m_start = lstart;
				m_nfa->m_end = rend;
			}

			// (s, e)* -> (ns - s, e - ns)
			void visit(const RepPtr& re) override {
				re->m_exp->accept(shared_from_this());
				NFANode* start = m_nfa->m_start;
				NFANode* end = m_nfa->m_end;

				m_nfa->m_start = new NFANode;
				m_nfa->m_end = m_nfa->m_start;
				m_nfa->m_nodes.push_back(m_nfa->m_start);
				m_nfa->m_start->addEdge(0, start);

				end->addEdge(0, m_nfa->m_end);

			}

			void visit(const GroupPtr& re) override {
				re->m_exp->accept(shared_from_this());
				m_nfa->m_start->m_group = re->m_group * 2;
				m_nfa->m_end->m_group = re->m_group * 2 + 1;
			}
		};
	}

	namespace NFAEngine {
		using namespace Parse;
		using namespace NFA;
		using namespace AST;

		// It's actually a NFA + eps-closure
		class NFAEngine {
		public:
			using str_size_t = std::string::size_type;

			NFAEngine(const std::string& str)
				: m_nfaCons(new NFAConstructor), m_nfa(m_nfaCons->construct(m_parse(str))) {}

			// Bad!
			// TODO: fix search
			std::string search(const std::string& str) {
				for (str_size_t i = 0; i < str.size(); ++i) {
					for (str_size_t j = 0; j < str.size() - i; ++j) {
						std::string tmpstr = str.substr(i, j);
						if (match(tmpstr))
							return tmpstr;
					}
				}
			}

			bool match(const std::string& str) {
				m_str = str;
				std::unordered_map<NFANode*, std::vector<int>>omap;
				std::unordered_map<NFANode*, std::vector<int>>nmap;

				std::vector<int> group = std::vector<int>(2 * m_nfa->m_maxGroup);
				omap[m_nfa->m_start] = group;
				addEpsilon(omap, m_nfa->m_start, group);

				// BFS DFA
				str_size_t i = 0;
				while (i < str.size() && !omap.empty()) {
					for (auto& s : omap) {
						NFANode* st = s.first;
						std::vector<int>& g = s.second;
						if (st->m_group != -1)
							g[st->m_group] = i;
						auto iter = st->m_edges.find('.');
						if (iter != st->m_edges.end()) {
							for (NFANode* e : iter->second) {
								nmap[e] = g;
								addEpsilon(nmap, e, g);
							}
						}
						iter = st->m_edges.find(str[i]);
						if (iter != st->m_edges.end()) {
							for (NFANode* e : iter->second) {
								nmap[e] = g;
								addEpsilon(nmap, e, g);
							}
						}
					}
					++i;
					omap.clear();
					std::swap(omap, nmap);
				}

				if (i < str.size())
					return false;
				auto iter = omap.find(m_nfa->m_end);
				if (iter != omap.end()) {
					m_group = iter->second;
					return true;
				}
				return false;
			}

		private:
			Parser m_parse;
			NFAConstructorPtr m_nfaCons;
			NFAPtr m_nfa;
			std::string m_str;
			std::vector<int> m_group;

			// eps-closure
			void addEpsilon(std::unordered_map<NFANode*, std::vector<int>>& nmap,
				NFANode* start, std::vector<int>& group, char v = 0) {
				std::queue<NFANode*> q;
				std::unordered_set<NFANode*> isVisited;
				q.push(start);
				isVisited.insert(start);

				while (!q.empty()) {
					NFANode* n = q.front();
					q.pop();
					auto iter = n->m_edges.find(v);
					if (iter == n->m_edges.end())
						continue;
					for (const auto& e : iter->second) {
						if (isVisited.find(e) != isVisited.end())
							continue;
						nmap[e] = group;
						q.push(e);
						isVisited.insert(e);
					}
				}
			}
		};

	}

}




#endif