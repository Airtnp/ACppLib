#ifndef SN_BUILTIN_OBSERVER_PTR
#define SN_BUILTIN_OBSERVER_PTR

#include "../sn_CommonHeader.h"

namespace sn_Builtin {
    
#ifdef SN_USE_REAL_OBSERVER_PTR
	namespace real_observer_ptr {
		template <typename T>
		class observer_ptr {
		public:
			using element_type = T;
			using pointer_type = T*;
			using reference_type = T&;

			constexpr observer_ptr() noexcept : m_ptr(nullptr) {}
			constexpr observer_ptr(std::nullptr_t) noexcept : m_ptr(nullptr) {}
			constexpr explicit observer_ptr(pointer_type p) noexcept : m_ptr(p) {}

			template <typename U>
			constexpr observer_ptr(const observer_ptr<U>& rhs) noexcept : m_ptr(rhs.get()) {}

			constexpr pointer_type get() const noexcept { return m_ptr; }
			constexpr reference_type operator*() const { 
				assert(m_ptr != nullptr);
				return *m_ptr;
			}
			constexpr pointer_type operator->() const noexcept {
				return m_ptr;
			}

			constexpr explicit operator bool() const noexcept { return m_ptr != nullptr; }
			constexpr explicit operator pointer_type() const noexcept { return m_ptr; }

			constexpr pointer_type release() noexcept {
				pointer_type p{m_ptr};
				reset();
				return p;
			}

			constexpr void reset(pointer_type p = nullptr) noexcept {
				m_ptr = p;
			}

			constexpr void swap(observer_ptr& rhs) noexcept {
				using std::swap;
				swap(m_ptr, rhs.m_ptr);
			}

		private:
			pointer_type m_ptr;

		};

		template <typename T>
		void swap(observer_ptr<T>& p1, observer_ptr<T>& p2) noexcept {
			p1.swap(p2);
		}

		template <typename T>
		observer_ptr<T> make_observer(T* p) noexcept {
			return observer_ptr<T>(p);
		}

		template<class T1, class T2>
		bool operator==(observer_ptr<T1> p1, observer_ptr<T2> p2) {
			return p1.get() == p2.get();
		}

		template<class T1, class T2>
		bool operator==(observer_ptr<T1> p1, std::nullptr_t) {
			return !p1;
		}
		
		template<class T1, class T2>
		bool operator==(std::nullptr_t, observer_ptr<T2> p2) {
			return !p2;
		}

		template<class T1, class T2>
		bool operator!=(observer_ptr<T1> p1, std::nullptr_t) {
			return static_cast<bool>(p1);
		}
		
		template<class T1, class T2>
		bool operator!=(std::nullptr_t, observer_ptr<T2> p2) {
			return static_cast<bool>(p2);
		}
	}
#endif
}

#endif