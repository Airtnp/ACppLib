#include "../sn_CommonHeader.h"

namespace sn_Assist {
    namespace sn_invoke {
		template <typename F, typename ...Args>
		auto invoke(F&& fn, Args&&... args) noexcept(noexcept(std::forward<F>(fn)(std::forward<Args>(args)...))) {
			return std::forward<F>(fn)(std::forward<Args>(args)...);
		}

		template <typename R, typename C, typename ...Args>
		auto invoke(R(C::*ptr_fn)(Args...), C* p, Args&&... args) noexcept(noexcept(p->ptr_fn(std::forward<Args>(args)...))) {
			return p->*ptr_fn(std::forward<Args>(args)...);
		}

		template <typename R, typename C, typename ...Args>
		auto invoke(R(C::*ptr_fn)(Args...), C&& c, Args&&... args) noexcept(noexcept(std::forward<C>(c).ptr_fn(std::forward<Args>(args)...))) {
				return std::forward<C>(c).*ptr_fn(std::forward<Args>(args)...);
		}

		template <typename R, typename ...Args>
		auto invoke(R(&fn)(Args...), Args&&... args) noexcept(noexcept(fn(std::forward<Args>(args)...))) {
			return fn(std::forward<Args>(args)...);
		}

		template <typename R, typename ...Args>
		auto invoke(R(*fn)(Args...), Args&&... args) noexcept(noexcept(fn(std::forward<Args>(args)...))) {
			return fn(std::forward<Args>(args)...);
		}

#define SN_INVOKE_GEN(SUFFIX) \
		template <typename R, typename C, typename ...Args> \
		auto invoke(R(C::*ptr_fn)(Args...) SUFFIX, C* p, Args&&... args) noexcept(noexcept(p->ptr_fn(std::forward<Args>(args)...))) { \
			return p->*ptr_fn(std::forward<Args>(args)...); \
		} \
		template <typename R, typename C, typename ...Args> \
		auto invoke(R(C::*ptr_fn)(Args...) SUFFIX, C&& c, Args&&... args) noexcept(noexcept(std::forward<C>(c).ptr_fn(std::forward<Args>(args)...))) { \
			return std::forward<C>(c).*ptr_fn(std::forward<Args>(args)...); \
		} \

#ifdef SN_ENABLE_SUSPICIOUS_IMPLEMENTATION

#define SN_INVOKE_GEN_2(SUFFIX) \
		template <typename R, typename ...Args> \
		auto invoke(R(&fn)(Args...) SUFFIX, Args&&... args) noexcept(noexcept(fn(std::forward<Args>(args)...))) { \
			return fn(std::forward<Args>(args)...); \
		} \
		template <typename R, typename ...Args> \
		auto invoke(R(*fn)(Args...) SUFFIX, Args&&... args) noexcept(noexcept(fn(std::forward<Args>(args)...))) { \
			return fn(std::forward<Args>(args)...); \
		} \

		SN_INVOKE_GEN_2(SN_EMPTY_ARGUMENT)
		SN_INVOKE_GEN_2(&)
		SN_INVOKE_GEN_2(&&)
		SN_INVOKE_GEN_2(const)
		SN_INVOKE_GEN_2(const &)
		SN_INVOKE_GEN_2(const &&)
		SN_INVOKE_GEN_2(volatile)
		SN_INVOKE_GEN_2(volatile &)
		SN_INVOKE_GEN_2(volatile &&)
		SN_INVOKE_GEN_2(const volatile)
		SN_INVOKE_GEN_2(const volatile &)
		SN_INVOKE_GEN_2(const volatile &&) 

#endif

		//For p::f() & and p::f() &&

		SN_INVOKE_GEN(SN_INVOKE_EMPTY_ARGUMENT)
		SN_INVOKE_GEN(&)
		SN_INVOKE_GEN(&&)
		SN_INVOKE_GEN(const)
		SN_INVOKE_GEN(const &)
		SN_INVOKE_GEN(const &&)
		SN_INVOKE_GEN(volatile)
		SN_INVOKE_GEN(volatile &)
		SN_INVOKE_GEN(volatile &&)
		SN_INVOKE_GEN(const volatile)
		SN_INVOKE_GEN(const volatile &)
		SN_INVOKE_GEN(const volatile &&)
	}
}