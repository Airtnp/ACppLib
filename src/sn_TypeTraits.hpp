#ifndef SN_TYPE_TRAITS_H
#define SN_TYPE_TRAITS_H

#include "sn_CommonHeader.h"

namespace sn_TypeTraits {
	namespace functor {
		struct is_functor_impl {
			template <typename T>
			static std::true_type check(decltype(&T::operator())*);
			template <typename T>
			static std::false_type check(...);
		};

		template <typename T>
		using is_functor = decltype(is_functor_impl::template check<T>(nullptr));

		template <typename T, bool = std::is_function<std::remove_pointer_t<T>>::value
			|| is_functor<T>::value>
			struct is_closure_impl {};

		template <typename T>
		struct is_closure_impl<T, true> : std::true_type {};
		template <typename T>
		struct is_closure_impl<T, false> : std::false_type {};

		template <typename T>
		struct is_closure : is_closure_impl<std::decay_t<T>> {};
	}
	
	namespace address {
		template <typename T>
		inline const T* addressofex(const T* t) {
			return t;
		}
		template <typename T>
		inline T* addressofex(T* t) {
			return t;
		}
		template <typename T>
		inline const T* addressofex(const T& t) {
			return std::addressof(t);
		}
		template <typename T>
		inline T* addressofex(T& t) {
			return std::addressof(t);
		}
	}
	// This is just llvm (remember the dynamic_cast)
	namespace polymorphic {
		template<typename T> char & is_polymorphic_impl(
			std::enable_if_t<
				sizeof(
					(T*) dynamic_cast<const volatile void*> (std::declval<T*>())
				) != 0, int>
		);
		template<typename T> int &is_polymorphic_impl(...);
		template <class T> 
		struct is_polymorphic : public std::integral_constant<bool, sizeof(is_polymorphic_impl<T>(0)) == 1> {};
	}
}











#endif