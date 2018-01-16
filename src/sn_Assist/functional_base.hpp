#include "../sn_CommonHeader.h"

namespace sn_Assist {
    namespace sn_functional_base {
		template <typename T>
		struct less_than {
			
			bool operator<=(const T& rhs) {
				return static_cast<T*>(this)->operator<(rhs) || static_cast<T*>(this)->operator==(rhs);
			}

			bool operator>(const T& rhs) {
				return !static_cast<T*>(this)->operator<(rhs);
			}

			bool operator>=(const T& rhs) {
				return !static_cast<T*>(this)->operator<(rhs) || static_cast<T*>(this)->operator==(rhs);
			}

			bool operator!=(const T& rhs) {
				return !static_cast<T*>(this)->operator==(rhs);
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

		//CRTP singleton (or just inherit)
		template <typename T>
		class singleton_base {
		public:
			static T& get_instance() {
				static T instance;
				return instance;
			}
		protected:
			singleton_base() {}
			~singleton_base() {}
		public:
			singleton_base(const singleton_base&) = delete;
			singleton_base& operator=(const singleton_base&) = delete;
		};

		// another implementation
		template <typename T>
		class singleton_of {
		private:
			static std::once_flag guard;
			static std::shared_ptr<T> ptr;
			singleton_of() = delete;
		public:
			static T* get_instance() {
				std::call_once(guard, []{ ptr = make_shared<T>() });
				return ptr;
			}
		}

		/* DCLP
		std::atomic<Singleton*> Singleton::m_instance;
		std::mutex Singleton::m_mutex;

		Singleton* Singleton::getInstance() {
			Singleton* tmp = m_instance.load(std::memory_order_relaxed);
			std::atomic_thread_fence(std::memory_order_acquire);
			// Or 
			// Singleton* tmp = m_instance.load(std::memory_order_acquire);
			// Singleton* tmp = m_instance.load(); SC atomics
			if (tmp == nullptr) {
				std::lock_guard<std::mutex> lock(m_mutex);
				tmp = m_instance.load(std::memory_order_relaxed);
				if (tmp == nullptr) {
					tmp = new Singleton;
					std::atomic_thread_fence(std::memory_order_release);
					m_instance.store(tmp, std::memory_order_relaxed);
					// Or 
					// m_instance.store(tmp, std::memory_order_release);
					// m_instance.store(tmp); SC 
				}
			}
			return tmp;
		}		
		*/

		template <typename T>
		std::once_flag singleton_of::guard;
		template <typename T>
		T* singleton_of::ptr;
	}
}