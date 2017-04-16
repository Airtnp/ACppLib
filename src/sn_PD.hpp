#ifndef SN_PD_H
#define SN_PD_H

#include "sn_CommonHeader.h"

// ref: https://github.com/dented42/derp/blob/master/memoization.rkt
// ref: https://github.com/dented42/derp/blob/master/fixed-points.rkt
// ref: https://github.com/dented42/derp/blob/master/lazy-structs.rkt
// ref: https://github.com/dented42/derp/blob/master/derp-core.rkt
// ref: https://github.com/tmmcguire/Java-Parser-Derivatives
// TODO: add hash parser -> add compact
namespace sn_PD {
	// Since std::hash<string> is not exists, use set<string>

	using StrSet = std::set<std::string>;

	class Parser {
	public:
		virtual std::shared_ptr<Parser> derive(char ch) = 0;
		virtual StrSet deriveNull() = 0;
	};

	using ParserPtr = std::shared_ptr<Parser>;
	

	class Empty : public Parser, public std::enable_shared_from_this<Empty> {
	public:
		std::shared_ptr<Parser> derive(char ch) override {
			return std::make_shared<Empty>();
		}
		StrSet deriveNull() override {
			return {};
		}
	};

	class Epsilon : public Parser, public std::enable_shared_from_this<Epsilon> {
		StrSet tree;
	public:
		Epsilon() { tree.insert(""); }
		Epsilon(char ch) { tree.insert(std::string(1, ch)); }
		std::shared_ptr<Parser> derive(char ch) override {
			return std::make_shared<Empty>();
		}
		StrSet deriveNull() override {
			return tree;
		}
	};

	class Literal : public Parser, public std::enable_shared_from_this<Literal> {
		char ch;
	public:
		Literal(char ch_) : ch(ch_) {}
		std::shared_ptr<Parser> derive(char ch_) override {
			if (ch == ch_)
				return std::make_shared<Epsilon>(ch);
			return std::make_shared<Empty>();
		}
		StrSet deriveNull() override {
			return {};
		}
	};

	class Alt : public Parser, public std::enable_shared_from_this<Alt> {
		ParserPtr p1;
		ParserPtr p2;
	public:
		Alt(ParserPtr p1_, ParserPtr p2_) : p1(p1_), p2(p2_) {}
		std::shared_ptr<Parser> derive(char ch) override {
			return std::make_shared<Alt>(p1->derive(ch), p2->derive(ch));
		}
		StrSet deriveNull() override {
			StrSet s1 = p1->deriveNull();
			StrSet s2 = p2->deriveNull();
			StrSet res;
			for (const auto& s : s1)
				res.insert(s);
			for (const auto& s : s2)
				res.insert(s);
			return res;
		}
	};

	class Delta : public Parser, public std::enable_shared_from_this<Delta> {
		ParserPtr p;
	public:
		Delta(ParserPtr p_) : p(p_) {}
		std::shared_ptr<Parser> derive(char ch) override {
			return std::make_shared<Empty>();
		}
		StrSet deriveNull() override {
			return p->deriveNull();
		}
	};

	class Concat : public Parser, public std::enable_shared_from_this<Concat> {
		ParserPtr p1;
		ParserPtr p2;
	public:
		Concat(ParserPtr p1_, ParserPtr p2_) : p1(p1_), p2(p2_) {}
		std::shared_ptr<Parser> derive(char ch) override {
			return std::make_shared<Alt>(
				std::make_shared<Concat>(p1->derive(ch), p2), 
				std::make_shared<Concat>(std::make_shared<Delta>(p1), p2->derive(ch))
				);
		}
		StrSet deriveNull() override {
			StrSet s1 = p1->deriveNull();
			StrSet s2 = p2->deriveNull();
			StrSet res;
			for (auto& s : s1) {
				for (auto& ss : s2) {
					res.insert(std::string("(") + s + ", " + ss + ")");
				}
			}
			return res;
		}
	};

	// Inherit it and transform deriveNull result
	template <typename T, typename U>
	struct Reduction {
		virtual U reduce(T t) = 0;
	};

	template <typename T, typename U>
	class Reduce : public Parser, public std::enable_shared_from_this<Reduce<T, U>> {
		ParserPtr p;
		Reduction<T, U> red;
	public:
		Reduce(ParserPtr p_, Reduction<T, U> red_) : p(p_), red(red_) {}
		std::shared_ptr<Parser> derive(char ch) override {
			return std::make_shared<Reduce>(p->derive(ch), red);
		}
		StrSet deriveNull() override {
			StrSet n;
			for (auto o : p->deriveNull())
				n.insert(red.reduce(o));
			return n;
		}
	};
	
	class Fix : public Parser, public std::enable_shared_from_this<Fix> {
		StrSet set;
		class Delay : public Parser, public std::enable_shared_from_this<Delay> {
			std::shared_ptr<Fix> p;
			char ch;
			ParserPtr deriv = nullptr;
			void force() {
				if (deriv == nullptr)
					deriv = p->innerDerive(ch);
			}
		public:
			Delay(std::shared_ptr<Fix> p_, char ch_) : p(p_), ch(ch_) {}
			ParserPtr derive(char ch) override {
				force();
				return deriv->derive(ch);
			}
			StrSet deriveNull() override {
				force();
				return deriv->deriveNull();
			}
		};
		std::map<std::pair<ParserPtr, char>, ParserPtr> derivs;
	public:
		virtual ParserPtr innerDerive(char ch);
		virtual StrSet innerDeriveNull();
		ParserPtr derive(char ch) override {
			if (derivs.end() != derivs.find(std::make_pair(shared_from_this(), ch)))
				derivs.insert(std::make_pair(std::make_pair(shared_from_this(), ch), std::make_shared<Delay>(shared_from_this(), ch)));
			
			return derivs.find(std::make_pair(shared_from_this(), ch))->second;
		}
		StrSet deriveNull() override {
			if (!set.empty())
				return set;
			StrSet n;
			do {
				set = n;
				n = innerDeriveNull();
			} while (!(set == n));
		}
	};

	class Recurrence : public Fix, public std::enable_shared_from_this<Recurrence> {
		ParserPtr p;
	public:
		void setParser(ParserPtr p_) {
			p = p_;
		}
		std::shared_ptr<Parser> innerDerive(char ch) override {
			return p->derive(ch);
		}
		StrSet innerDeriveNull() override {
			return p->deriveNull();
		}
	};

}





#endif