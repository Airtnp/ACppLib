#include "../sn_CommonHeader.h"

namespace sn_Assist {
    namespace sn_functional_base {
		template <typename T>
		class less_than {
			
			bool operator<=(const T& rhs) {
				return static_cast<T*>(this)->operator<(rhs) || static_cast<T*>(this)->operator=(rhs);
			}

			bool operator>(const T& rhs) {
				return !static_cast<T*>(this)->operator<(rhs);
			}

			bool operator>=(const T& rhs) {
				return !static_cast<T*>(this)->operator<(rhs) || static_cast<T*>(this)->operator=(rhs);
			}

			bool operator!=(const T& rhs) {
				return !static_cast<T*>(this)->operator=(rhs);
			}

		};

		class noncopyable {
		private:
			noncopyable(const noncopyable&) = delete;
			noncopyable& operator=(const noncopyable&) = delete;
		};

		class nonmoveable {
		private:
			nonmoveable(nonmoveable&&) = delete;
			nonmoveable& operator=(nonmoveable&&) = delete;
		};

		class noncopymoveable : public noncopyable, public nonmoveable {};

		class nondirectconstructable {
		protected:
			void* operator new(std::size_t size) {
				return ::operator new(size);
			}
			void operator delete(void* ptr) {
				::operator delete(ptr);
			}
		};

		// default_constructor
		template <bool dc>
		struct enable_default_constructor {};

		template <>
		struct enable_default_constructor<true> {
			using enable_type = enable_default_constructor;
			enable_default_constructor() = default;
			constexpr enable_default_constructor(void*) {}
		};
		template <>
		struct enable_default_constructor<false> {
			using enable_type = enable_default_constructor;
			enable_default_constructor() = delete;
			constexpr enable_default_constructor(void*) {}
		};
	}
}