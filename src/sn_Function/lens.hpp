#ifndef SN_FUNCTION_LENS_H
#define SN_FUNCTION_LENS_H

#include <functional>

namespace sn_Function {
    namespace lens {
		template <typename V, typename F>
		struct Lens {
			std::function<F(V)> getter;
			std::function<V(V, F)> setter;

			V apply(const V& v, const std::function<F(F)>& trans) const {
				auto z = getter(value);
				return setter(v, trans(z));
			}
		};

        template <typename V, typename F>
		Lens<V, F> make_lens(const std::function<F(V)>& getter, const std::function<V(V, F)>& setter) {
			Lens<V, F> lens;
			lens.getter = getter;
			lens.setter = setter;
			return lens;
		}

		template <typename ...Ls>
		struct LensStack {
			template <typename Reroll, typename Lx>
			void reroll_impl(Reroll& rerolled, const Lx& lx) const {
				rerolled.m_lens = lx;
			}
			template <typename F>
			F get(const V& f) const {
				return f;
			}
			template <typename F>
			F apply(const F& value, const std::function<F(F)>& trans) const {
				return trans(value);
			}
		};

		template <typename L, typename ...Ls>
		struct LensStack<L, Ls...> : LensStack<Ls...> {
			using base_type = LensStack<Ls...>;
			LensStack(L lens, Ls... tail)
				: LensStack<Ls...>(tail...),
				m_lens(lens) {}
			
			template <typename Reroll, typename Lx>
			void reroll_impl(Reroll& rerolled, const Lx& lx) const {
				rerolled.m_lens = m_lens;
				m_base.reroll(rerolled.m_base, lx);
			}

			template <typename Lx>
			LensStack<L, Ls..., Lx> reroll(const Lx& lx) const {
				LensStack<L, Ls..., Lx> rerolled;
				rerolled.m_lens = m_lens;
				m_base.reroll_impl(rerolled.m_base, lx);
				return rerolled;
			}

			template <typename V>
			auto get(const V& value) const {
				auto z = m_lens.getter(value);
				return m_base.get(z);
			}

			template <typename V, typename F>
			V apply(const V& value, const std::function<F(F)>& trans) const {
				auto z = m_lens.getter(value);
				return m_lens.setter(value, m_base.apply(z, trans));
			}
			
			base_type& m_base = static_cast<base_type&>(*this);
			L m_lens;
			
		}

		// Implement of infix
		struct Proxy {};
		constexpr const Proxy proxy;
		template <typename V, typename F>
		LensStack<Lens<V, F>> operator< (const Lens<V, F>& lens, const Proxy&) {
			return LensStack<Lens<V, F>>(lens);
		}

		template <typename LS, typename L>
		typename LS::template reroll_type<L> operator> (const LS& stack, const L& lens) {
			return stack.reroll(lens);
		}

		#define to < sn_Function::lens::proxy >

		template <typename V, typename F>
		F view(const Lens<V, F>& lens, const V& v) {
			return lens.getter(v);
		}

		template <typename LS, typename V>
		auto view(const LS& ls, const V& v) {
			return ls.get(v);
		}

		template <typename V, typename F>
		V set(const Lens<V, F>& lens, const V& v, const F& f) {
			return lens.setter(v, f);
		}

		template <typename LS, typename V, typename F>
		V set(const LS& ls, const V& v, const F& f) {
			return ls.apply(v, [=](F){ return f; });
		}

		template <typename V, typename F>
		V over(const Lens<V, F>& lens, const V& v, const std::function<F(F)>& trans) {
			auto z = lens.getter(v);
			return lens.setter(v, trans(z));
		}

		template <typename LS, typename V, typename F>
		V set(const LS& ls, const V& v, const std::function<F(F)>& trans) {
			return ls.apply(v, trans);
		}
	}
}


#endif