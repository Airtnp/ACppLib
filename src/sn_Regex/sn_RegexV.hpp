#ifndef SN_REGEX_V_H
#define SN_REGEX_V_H

#include "sn_CommonHeader.h"

// ref: Ninputer/VBF
// Totally pass-by-value... unlike C#, it's hard to manager reference
// @TODO: https://zhuanlan.zhihu.com/p/32896848
namespace sn_RegexV {

	namespace REN {

		enum class RegularExpressionType {
			Empty, Symbol, Alternation, Concatenation, KleeneStar, AlternationCharSet, StringLiteral,
		};

		using RET = RegularExpressionType;

		template <typename T>
		class RegularExpressionConverter;

		class RegularExpression {
		public:
			template <typename T>
			virtual T Accept(RegularExpressionConverter<T> converter) {}

			RET ExpressionType() {
				return m_expType;
			}

			static RegularExpression Symbol(char c) {
				return SymbolExpression(c);
			}
			RegularExpression Many() {
				if (m_expType == RegularExpressionType::KleeneStar)
					return *this;
				return KleeneStarExpression(*this);
			}
			RegularExpression Concat(RegularExpression follow) {
				return ConcatenationExpression(*this, follow);
			}
			RegularExpression Union(RegularExpression rhs) {
				if (this == std::addressof(rhs))
					return *this;
				return AlternationExpression(*this, rhs);
			}
			static RegularExpression Literal(std::string literal) {
				return StringLiteralExpression(literal);
			}
			static RegularExpression CharSet(const char* charSet) {
				return AlternationCharSetExpression(charSet);
			}
			static RegularExpression Empty() {
				return EmptyExpression();
			}

			RegularExpression Many1() {
				return this->Concat(this->Many());
			}
			RegularExpression Optional() {
				return this->Union(Empty());
			}
			static RegularExpression Range(char min, char max) {
				std::vector<char> rangeCharSet;
				for (char c = min; c <= max; ++c) {
					rangeCharSet.push_back(c);
				}
				return AlternationCharSetExpression(rangeCharSet);
			}
			static RegularExpression CharsOf(std::function<bool(char)> charPredicate) {
				std::vector<char> rangeCharSet;
				for (char c = CHAR_MIN; c <= CHAR_MAX; ++c) {
					if (charPredicate(c))
						rangeCharSet.push_back(c);
				}
				return AlternationCharSetExpression(rangeCharSet);
			}


		protected:
			RegularExpression(RegularExpressionType expType) {
				m_expType = expType;
			}

		private:
			RegularExpressionType m_expType;

		};

		RegularExpression operator|(RegularExpression l, RegularExpression r) {
			return AlternationExpression(l, r);
		}
		RegularExpression operator+(RegularExpression l, RegularExpression r) {
			return ConcatenationExpression(l, r);
		}


		class EmptyExpression final : public RegularExpression {
		public:
			EmptyExpression() : RegularExpression(RET::Empty) {}
		};

		class AlternationExpression final : public RegularExpression {
		public:
			AlternationExpression(RegularExpression exp1, RegularExpression exp2)
				: m_exp1(exp1), m_exp2(exp2), RegularExpression(RET::Alternation) {}
			RegularExpression Expression1() {
				return m_exp1;
			}
			RegularExpression Expression2() {
				return m_exp2;
			}
			template <typename T>
			T Accept(RegularExpressionConverter<T> converter) {
				return converter.ConverterAlternation(*this);
			}
		private:
			RegularExpression m_exp1;
			RegularExpression m_exp2;
		};

		class ConcatenationExpression final : public RegularExpression {
		public:
			ConcatenationExpression(RegularExpression exp1, RegularExpression exp2)
				: m_expL(exp1), m_expR(exp2), RegularExpression(RET::Concatenation) {}
			RegularExpression ExpressionL() {
				return m_expL;
			}
			RegularExpression ExpressionR() {
				return m_expR;
			}
			template <typename T>
			T Accept(RegularExpressionConverter<T> converter) {
				return converter.ConvertConcatenation(*this);
			}
		private:
			RegularExpression m_expL;
			RegularExpression m_expR;
		};

		class KleeneStarExpression final : public RegularExpression {
		public:
			KleeneStarExpression(RegularExpression innerExp)
				: RegularExpression(RET::KleeneStar), m_exp(innerExp) {}
			RegularExpression InnerExpression() {
				return m_exp;
			}
			template <typename T>
			T Accept(RegularExpressionConverter<T> converter) {
				return converter.ConvertKleeneStar(*this);
			}
		private:
			RegularExpression m_exp;
		};

		class SymbolExpression final : public RegularExpression {
		public:
			SymbolExpression(char symbol) : RegularExpression(RegularExpressionType::Symbol), m_symbol(symbol) {}

			char Symbol() {
				return m_symbol;
			}

			template <typename T>
			T Accept(RegularExpressionConverter<T> converter) {
				return converter.ConvertSymbol(*this);
			}
		private:
			char m_symbol;
		};

		class AlternationCharSetExpression final : public RegularExpression {
		public:
			AlternationCharSetExpression(std::vector<char> charSet)
				: RegularExpression(RegularExpressionType::AlternationCharSet), m_charSet(charSet) {}
			AlternationCharSetExpression(const char* charSet)
				: RegularExpression(RegularExpressionType::AlternationCharSet) {
				const char* p = charSet;
				while (*p != '\0') {
					m_charSet.push_back(*p);
					++p;
				}
			}


			std::vector<char> CharSet() {
				return m_charSet;
			}

			template <typename T>
			T Accept(RegularExpressionConverter<T> converter) {
				return converter.ConvertAlternationCharSet(*this);
			}
		private:
			std::vector<char> m_charSet;
		};

		class StringLiteralExpression final : public RegularExpression {
		public:
			StringLiteralExpression(std::string str)
				: RegularExpression(RegularExpressionType::StringLiteral), m_string(str) {}

			std::string String() {
				return m_string;
			}

			template <typename T>
			T Accept(RegularExpressionConverter<T> converter) {
				return converter.ConvertStringLiteral(*this);
			}
		private:
			std::string m_string;
		};

		template <typename T>
		class RegularExpressionConverter {
		public:
			T Convert(RegularExpression exp) {
				if (exp == NULL)
					return T();
				return exp.Accept(*this);
			}
			virtual T ConvertAlternation(AlternationExpression) = 0;
			virtual T ConvertSymbol(SymbolExpression) = 0;
			virtual T ConvertEmpty(EmptyExpression) = 0;
			virtual T ConvertConcatenation(ConcatenationExpression) = 0;
			virtual T ConvertAlternationCharSet(AlternationCharSetExpression) = 0;
			virtual T ConvertStringLiteral(StringLiteralExpression) = 0;
			virtual T ConvertKleeneStar(KleeneStarExpression) = 0;
		protected:
			RegularExpressionConverter() {}
		};

	}

	namespace NFA {
		using namespace REN;

		class CharTable {
		public:
			CharTable() {}
			CharTable(std::vector<unsigned short> ct, unsigned short maxIndex)
				: m_maxIndex(maxIndex), m_charTable(ct) {}
			CharTable& operator=(const CharTable& rhs) {
				m_maxIndex = rhs.m_maxIndex;
				m_minIndex = rhs.m_minIndex;
				m_charTable = rhs.m_charTable;
			}
			unsigned short MinIndex() {
				return m_minIndex;
			}
			unsigned short MaxIndex() {
				return m_maxIndex;
			}
			unsigned short GetCompactClass(char c) {
				return m_charTable[static_cast<std::size_t>(c)];
			}
			bool HasCompactClass(char c) {
				return m_charTable[static_cast<std::size_t>(c)] >= m_minIndex;
			}
		private:
			unsigned short m_maxIndex;
			unsigned short m_minIndex = 1;
			std::vector<unsigned short> m_charTable;
		};

		class NFAState;

		class NFAEdge {
		public:
			NFAEdge() {}
			NFAEdge(int symbol, NFAState targetState)
				: m_symbol(symbol), m_state(targetState) {}
			NFAEdge(NFAState targetState)
				: m_state(targetState), m_symbol(-1) {}
			NFAEdge& operator=(const NFAEdge& ne) {
				if (this == std::addressof(ne))
					return *this;
				m_symbol = ne.m_symbol;
				m_state = ne.m_state;
				return *this;
			}
			NFAState TargetState() {
				return m_state;
			}
			int Symbol() {
				return m_symbol;
			}
			bool IsEmpty() {
				return m_symbol == -1;
			}

		private:
			int m_symbol;  // better optional
			NFAState m_state;
		};

		class NFAState {
		public:
			NFAState()
				: m_tokenIndex(-1) {}
			NFAState& operator=(const NFAState& ns) {
				if (this == std::addressof(ns))
					return *this;
				m_index = ns.m_index;
				m_tokenIndex = ns.m_tokenIndex;
				m_outEdges = ns.m_outEdges;
				return *this;
			}
			std::vector<NFAEdge> OutEdges() {
				return m_outEdges;
			}
			void AddEmptyEdgeTo(NFAState& targetState) {
				m_outEdges.push_back(NFAEdge(targetState));
			}
			void AddEdge(NFAEdge& edge) {
				m_outEdges.push_back(edge);
			}
			int m_index;
			int m_tokenIndex;
		private:
			std::vector<NFAEdge> m_outEdges;
		};

		class NFAModel {
		public:
			NFAModel() {}
			std::vector<NFAState>& States() {
				return m_states;
			}
			void AddState(NFAState& state) {
				state.m_index = m_states.size();
				m_states.push_back(state);
			}
			void AddStates(std::vector<NFAState>& states) {
				for (auto& s : states)
					AddState(s);
			}
			NFAState m_tailState;
			NFAEdge m_entryEdge;
		private:
			std::vector<NFAState> m_states;
		};

		class NFAConverter : public RegularExpressionConverter<NFAModel> {
		public:
			NFAConverter() {}
			NFAConverter(CharTable& ct)
				: m_charTable(ct) {}

			NFAModel ConvertAlternation(AlternationExpression exp) override {
				auto nfa1 = Convert(exp.Expression1());
				auto nfa2 = Convert(exp.Expression2());

				NFAState head = NFAState();
				NFAState tail = NFAState();

				head.AddEdge(nfa1.m_entryEdge);
				head.AddEdge(nfa2.m_entryEdge);
				nfa1.m_tailState.AddEmptyEdgeTo(tail);
				nfa2.m_tailState.AddEmptyEdgeTo(tail);

				NFAModel alternationNFA = NFAModel();
				alternationNFA.AddState(head);
				alternationNFA.AddStates(nfa1.States());
				alternationNFA.AddStates(nfa2.States());
				alternationNFA.AddState(tail);

				alternationNFA.m_entryEdge = NFAEdge(head);
				alternationNFA.m_tailState = tail;

				return alternationNFA;
			}

			NFAModel ConvertSymbol(SymbolExpression exp) override {
				NFAState tail = NFAState();
				unsigned short cclass = m_charTable.GetCompactClass(exp.Symbol());
				NFAEdge entryEdge = NFAEdge(cclass, tail);

				NFAModel symbolNFA = NFAModel();
				symbolNFA.AddState(tail);
				symbolNFA.m_tailState = tail;
				symbolNFA.m_entryEdge = entryEdge;

				return symbolNFA;
			}

			NFAModel ConvertEmpty(EmptyExpression exp) override {
				NFAState tail = NFAState();
				NFAEdge entryEdge = NFAEdge(tail);

				NFAModel emptyNFA = NFAModel();
				emptyNFA.AddState(tail);
				emptyNFA.m_tailState = tail;
				emptyNFA.m_entryEdge = entryEdge;

				return emptyNFA;
			}

			NFAModel ConvertConcatenation(ConcatenationExpression exp) override {
				auto leftNFA = Convert(exp.ExpressionL());
				auto rightNFA = Convert(exp.ExpressionR());
				leftNFA.m_tailState.AddEdge(rightNFA.m_entryEdge);

				auto concatenationNFA = NFAModel();
				concatenationNFA.AddStates(leftNFA.States());
				concatenationNFA.AddStates(rightNFA.States());
				concatenationNFA.m_entryEdge = leftNFA.m_entryEdge;
				concatenationNFA.m_tailState = rightNFA.m_tailState;

				return concatenationNFA;
			}

			NFAModel ConvertAlternationCharSet(AlternationCharSetExpression exp) override {
				NFAState head = NFAState();
				NFAState tail = NFAState();

				NFAModel charSetNFA = NFAModel();
				charSetNFA.AddState(head);

				std::unordered_set<unsigned short> cclassSet{};
				for (const auto& s : exp.CharSet()) {
					unsigned short cclass = m_charTable.GetCompactClass(s);
					if (cclassSet.insert(s).second) {
						auto symbolEdge = NFAEdge(cclass, tail);
						head.AddEdge(symbolEdge);
					}
				}

				charSetNFA.AddState(tail);
				charSetNFA.m_entryEdge = NFAEdge(head);
				charSetNFA.m_tailState = tail;

				return charSetNFA;
			}

			NFAModel ConvertStringLiteral(StringLiteralExpression exp) override {
				NFAModel literalNFA = NFAModel();
				NFAState lastState;
				bool firstNull = false;
				for (const auto& s : exp.String()) {
					auto symbolState = NFAState();
					unsigned short cclass = m_charTable.GetCompactClass(s);
					auto symbolEdge = NFAEdge(cclass, symbolState);
					if (firstNull)
						lastState.AddEdge(symbolEdge);
					else {
						literalNFA.m_entryEdge = symbolEdge;
						firstNull = true;
					}
					lastState = symbolState;
					literalNFA.AddState(symbolState);
				}
				literalNFA.m_tailState = lastState;

				return literalNFA;
			}

			NFAModel ConvertKleeneStar(KleeneStarExpression exp) override {
				auto innerNFA = Convert(exp.InnerExpression());
				auto newTail = NFAState();
				auto entry = NFAEdge(newTail);

				innerNFA.m_tailState.AddEmptyEdgeTo(newTail);
				newTail.AddEdge(innerNFA.m_entryEdge);

				auto kleeneStarNFA = NFAModel();
				kleeneStarNFA.AddStates(innerNFA.States());
				kleeneStarNFA.AddState(newTail);
				kleeneStarNFA.m_entryEdge = entry;
				kleeneStarNFA.m_tailState = newTail;

				return kleeneStarNFA;
			}

		private:
			CharTable m_charTable;
		};

	}

	namespace LEX {
		using REN::RegularExpression;
		using NFA::NFAModel;
		using NFA::NFAConverter;
		using NFA::CharTable;

		class Token {
		public:
			Token(int index, int lexerIndex)
				: m_index(index), m_lexerIndex(lexerIndex) {}
			Token(int index, std::string description, int lexerIndex)
				: m_index(index), m_desc(description), m_lexerIndex(lexerIndex) {}
			int Index() { return m_index; }
			int LexerIndex() { return m_lexerIndex; }
			std::string Description() { return m_desc; }

			bool operator==(Token& rhs) {
				return Index() == rhs.Index();
			}

		private:
			int m_index;
			int m_lexerIndex;
			std::string m_desc;
		};

		class Lexicon;
		class Lexer;

		class TokenInfo {
		public:
			TokenInfo(RegularExpression def, Lexicon lex, Lexer state, Token tag)
				: m_re(def), m_lexicon(lex), m_lexer(state), m_tag(tag) {}

			NFAModel CreateFiniteAutomatonModel(NFAConverter converter) {
				NFAModel NFA = converter.Convert(m_re);
				NFA.m_tailState.m_tokenIndex = m_tag.Index();
				return NFA;
			}

			Token& Tag() { return m_tag; }
			Lexicon& TLexicon() { return m_lexicon; }
			Lexer& State() { return m_lexer; }
			RegularExpression& Definition() { return m_re; }
		private:
			Token m_tag;
			Lexicon m_lexicon;
			Lexer m_lexer;
			RegularExpression m_re;
		};

		class Lexer {
		public:
			Lexer(Lexicon lexicon, int index)
				: m_lexicon(lexicon), m_index(index), m_level(0) {}
			Lexer(Lexicon lexicon, int index, Lexer& baseLexer)
				: m_lexicon(lexicon), m_index(index), m_baseLexer(std::addressof(baseLexer)) {
				m_level = baseLexer.Level() + 1;
				baseLexer.m_children.push_back(*this);
			}
			int Level() { return m_level; }
			int Index() { return m_index; }
			Lexicon& TLexicon() { return m_lexicon; }
			Lexer& BaseLexer() { return *m_baseLexer; }
			std::vector<Lexer>& Children() { return m_children; }

			Token DefineToken(RegularExpression regex, std::string description) {
				int indexInState = m_tokens.size();
				TokenInfo token = m_lexicon.AddToken(regex, *this, indexInState, description);
				m_tokens.push_back(token);
				return token.Tag();
			}
			Token DefineToken(RegularExpression regex) {
				int indexInState = m_tokens.size();
				TokenInfo token = m_lexicon.AddToken(regex, *this, indexInState);
				m_tokens.push_back(token);
				return token.Tag();
			}
			Lexer CreateSubLexer() {
				return m_lexicon.DefineLexer(*this);
			}
		private:
			std::vector<TokenInfo> m_tokens;
			Lexicon m_lexicon;
			Lexer* m_baseLexer;
			int m_index;
			int m_level;
			std::vector<Lexer> m_children;
		};


		class Lexicon {
		public:
			Lexicon()
				: m_defaultState(*this, 0) {}
			Lexer TLexer() { return m_defaultState; }
			int LexerCount() { return m_lexerStates.size(); }
			int TokenCount() { return m_tokenList.size(); }
			TokenInfo AddToken(RegularExpression definition, Lexer state, int indexInState, std::string description) {
				int index = m_tokenList.size();
				Token tag = Token(index, description, state.Index());
				TokenInfo token = TokenInfo(definition, *this, state, tag);
				m_tokenList.push_back(token);
				return token;
			}
			TokenInfo AddToken(RegularExpression definition, Lexer state, int indexInState) {
				int index = m_tokenList.size();
				Token tag = Token(index, state.Index());
				TokenInfo token = TokenInfo(definition, *this, state, tag);
				m_tokenList.push_back(token);
				return token;
			}
			const std::vector<Lexer> GetLexers() { return m_lexerStates; }
			const std::vector<TokenInfo> GetTokens() { return m_tokenList; }
			Lexer DefineLexer(Lexer& baseLexer) {
				int index = m_lexerStates.size();
				Lexer newState = Lexer(*this, index, baseLexer);
				m_lexerStates.push_back(newState);
				return newState;
			}
			// Temporarily non-compact (so DFA will scan 1,2,....,n
			CharTable CreateCompactCharSetManager() {
				std::vector<unsigned short> v;
				for (std::size_t i = 1; i <= 255; ++i)
					v.push_back(i);
				return CharTable(v, 255);
			}
		private:
			const Lexer m_defaultState;
			std::vector<Lexer> m_lexerStates;
			std::vector<TokenInfo> m_tokenList;
		};
	}


	namespace DFA {
		using NFA::NFAModel;
		using NFA::NFAState;
		using NFA::NFAEdge;
		using NFA::NFAConverter;
		using LEX::Lexicon;
		using LEX::Lexer;
		using LEX::CharTable;
		using LEX::TokenInfo;
		using LEX::Token;

		class DFAState;

		class DFAEdge {
		public:
			DFAEdge() {}
			DFAEdge(int symbol, DFAState targetState)
				: m_symbol(symbol), m_state(targetState) {}
			DFAEdge& operator=(const DFAEdge& ne) {
				if (this == std::addressof(ne))
					return *this;
				m_symbol = ne.m_symbol;
				m_state = ne.m_state;
				return *this;
			}
		private:
			int m_symbol;
			DFAState m_state;
		};

		class DFAState {
		public:
			DFAState() {}
			const std::vector<DFAEdge> OutEdges() {
				return m_edges;
			}
			std::unordered_set<int>& NFAStateSet() {
				return m_NFAStates;
			}
			void AddEdge(DFAEdge edge) {
				m_edges.push_back(edge);
			}
			int m_index;
		private:
			std::unordered_set<int> m_NFAStates;
			std::vector<DFAEdge> m_edges;
		};

		class DFAModel {
		public:
			DFAModel(Lexicon lexicon)
				: m_lexicon(lexicon) {
				int stateCount = lexicon.LexerCount();
				m_acceptTables.resize(stateCount);
			}
			CharTable CompactCharSetManager() { return m_charTable; }
			const std::vector<DFAState> States() { return m_DFAStates; }
			std::vector<std::vector<int>> GetAcceptTables() { return m_acceptTables; }

			static DFAModel Create(Lexicon lexicon) {
				DFAModel newDFA = DFAModel(lexicon);
				newDFA.ConvertLexiconToNFA();
				newDFA.ConvertNFAToDFA();
				return newDFA;
			}

			void ConvertLexiconToNFA() {
				m_charTable = m_lexicon.CreateCompactCharSetManager();
				NFAConverter converter = NFAConverter(m_charTable);
				NFAState entryState = NFAState();
				NFAModel lexerNFA = NFAModel();
				lexerNFA.AddState(entryState);
				
				for (auto token : m_lexicon.GetTokens()) {
					NFAModel tokenNFA = token.CreateFiniteAutomatonModel(converter);
					entryState.AddEdge(tokenNFA.m_entryEdge);
					lexerNFA.AddStates(tokenNFA.States());
				}
				lexerNFA.m_entryEdge = NFAEdge(entryState);
				m_NFA = lexerNFA;
			}

			void ConvertNFAToDFA() {
				auto NFAStates = m_NFA.States();
				DFAState state0 = DFAState();
				AddDFAState(state0);

				DFAState preState1 = DFAState();
				int NFAStartIndex = m_NFA.m_entryEdge.TargetState().m_index;
				preState1.NFAStateSet().insert(NFAStartIndex);
				DFAState state1 = GetClosure(preState1);
				AddDFAState(state1);

				int p = 1, j = 0;
				std::vector<DFAState> newStates(m_charTable.MaxIndex() + 1);
				while (j <= p) {
					auto sourceState = m_DFAStates[j];
					for (int symbol = m_charTable.MinIndex(); symbol < m_charTable.MaxIndex() + 1; ++symbol) {
						DFAState e = GetDFAState(sourceState, symbol);
						newStates[symbol] = e;
					}
					for (int symbol = m_charTable.MinIndex(); symbol < m_charTable.MaxIndex() + 1; ++symbol) {
						DFAState e = newStates[symbol];
						bool isSetExist = false;
						for (int i = 0; i < p + 1; ++i) {
							if (e.NFAStateSet() == m_DFAStates[i].NFAStateSet()) {
								DFAEdge newEdge = DFAEdge(symbol, m_DFAStates[i]);
								sourceState.AddEdge(newEdge);
								isSetExist = true;
							}
						}
						if (!isSetExist) {
							p += 1;
							AddDFAState(e);
							DFAEdge newEdge = DFAEdge(symbol, e);
							sourceState.AddEdge(newEdge);
						}
					}
					++j;
				}


			}

		private:
			std::vector<int> AppendEosToekn(std::vector<int> list) {
				list.push_back(m_lexicon.TokenCount());
				return list;
			}

			void SetAcceptState(int lexerStateIndex, int DFAStateIndex, int tokenIndex) {
				m_acceptTables[lexerStateIndex][DFAStateIndex] = tokenIndex;
			}

			void AddDFAState(DFAState state) {
				state.m_index = m_DFAStates.size();
				m_DFAStates.push_back(state);
				for (int i = 0; i < m_acceptTables.size(); ++i) {
					m_acceptTables[i].push_back(-1);
				}
				auto tokens = m_lexicon.GetTokens();
				auto lexerStates = m_lexicon.GetLexers();
				std::vector<TokenInfo> v;
				for (const auto& i : state.NFAStateSet()) {
					auto tokenIndex = m_NFA.States()[i].m_tokenIndex;
					if (tokenIndex >= 0) {
						auto token = tokens[tokenIndex];
						v.push_back(token);
					}
				}
				std::sort(v.begin(), v.end(), [](TokenInfo v1, TokenInfo v2) {
					return v1.Tag().Index() < v2.Tag().Index();
				});
				std::vector<std::tuple<int, TokenInfo>> acceptStates;
				for (auto p : v) {
					acceptStates.push_back(std::make_tuple(p.State().Index(), p));
				}
				std::sort(acceptStates.begin(), acceptStates.end(), [&lexerStates](std::tuple<int, TokenInfo> l1, std::tuple<int, TokenInfo> l2) {
					return lexerStates[std::get<0>(l1)].Level() < lexerStates[std::get<0>(l2)].Level();
				});
				if (acceptStates.size() > 0) {
					std::queue<Lexer> stateTreeQueue;
					for (auto p : acceptStates) {
						int acceptTokenIndex = std::get<1>(p).Tag().Index();
						stateTreeQueue.push(lexerStates[std::get<0>(p)]);
						while (stateTreeQueue.size() > 0) {
							auto currentLexerState = stateTreeQueue.front();
							stateTreeQueue.pop();
							for (auto child : currentLexerState.Children()) {
								stateTreeQueue.push(child);
							}
							SetAcceptState(currentLexerState.Index(), state.m_index, acceptTokenIndex);
						}
					}
					
				}
			}

			DFAState GetDFAState(DFAState start, int symbol) {
				DFAState target = DFAState();
				auto NFAStates = m_NFA.States();
				for (auto s : start.NFAStateSet()) {
					NFAState nfaState = NFAStates[s];
					auto outEdges = nfaState.OutEdges();
					int edgeCount = outEdges.size();
					for (int i = 0; i < edgeCount; ++i) {
						auto edge = outEdges[i];
						if (!edge.IsEmpty() && symbol == edge.Symbol()) {
							int targetIndex = edge.TargetState().m_index;
							target.NFAStateSet().insert(targetIndex);
						}
					}
				}
				return GetClosure(target);
			}

			DFAState GetClosure(DFAState state) {
				DFAState closure = DFAState();
				auto nfaStates = m_NFA.States();
				for (auto&& s : state.NFAStateSet()) {  // merge in c++17
					closure.NFAStateSet().insert(s);
				}
				bool changed = true;
				while (changed) {
					changed = false;
					std::vector<int> lastStateSet(closure.NFAStateSet().size());
					for (auto&& s : closure.NFAStateSet()) {  // extract in c++17
						lastStateSet.push_back(s);
					}
					for (const auto& stateIndex : lastStateSet) {
						NFAState nfaState = nfaStates[stateIndex];
						auto outEdges = nfaState.OutEdges();
						int edgeCount = outEdges.size();
						for (int i = 0; i < edgeCount; ++i) {
							auto edge = outEdges[i];
							if (!edge.IsEmpty()) {
								NFAState target = edge.TargetState();
								int targetIndex = target.m_index;
								changed = closure.NFAStateSet().insert(targetIndex).second || changed;
							}
						}
					}
				}
				return closure;
			}

			std::vector<std::vector<int>> m_acceptTables;
			std::vector<DFAState> m_DFAStates;
			Lexicon m_lexicon;
			NFAModel m_NFA;
			CharTable m_charTable;
		};


	}

	// Parser Incomplete... C# is so different with C++
	namespace SCAN {
		using LEX::Lexicon;
		using DFA::DFAModel;

		class Lexeme {

		};

		class FiniteAutomationEngin {

		};

		class CompressedTransitionTable {

		};

		class Scanner {

		};

		/*
		Scanner make_scanner(Lexicon lexicon) {
			DFAModel dfa = DFAModel::Create(lexicon);
			
		}*/

	}


	using RE = REN::RegularExpression;
}






#endif