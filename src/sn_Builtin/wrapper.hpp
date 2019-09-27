#include "../sn_CommonHeader.h"

namespace sn_Builtin {
    // TODO: add unwrap things
	// ref: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0318r0.pdf
	namespace pointer_wrapper {
		// Support incomplete type (result_of_t cannot)
		// ref: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0357r0.html
		template<typename T>
		class reference_wrapper {
			T* ptr;
		public:
			using type = T;
			reference_wrapper(T& val) noexcept
				: ptr(std::addressof(val)) {}
			reference_wrapper(T&&) = delete;
			T& get() const noexcept { return *ptr; }
			operator T&() const noexcept { return *ptr; }
			template<typename... Args>
			auto operator()(Args&&... args)
				-> std::result_of_t<T&(Args...)> {
				return std::invoke(*ptr, std::forward<Args>(args)...);
			}
		};

		template <typename T>
		class rvalue_reference_wrapper {
		private:
			T&& val;
		public:
			explicit rvalue_reference_wrapper(T&& v)
				: val(std::move(v)) {}
			explicit operator T() {
				return T{std::move(val)};
			}
		};

		template <typename T>
		typename std::enable_if_t<!std::is_lvalue_reference<T>::value, rvalue_reference_wrapper<T>> rref(T&& v) {
			return rvalue_reference_wrapper<T>{std::move(v)};
		}

		template <typename T>
		void rref(T&) = delete;

		/*
			// ref: https://stackoverflow.com/questions/8468774/can-i-list-initialize-a-vector-of-move-only-type
			elements of an initializer list are always passed via const-reference.

		Usage:
			std::vector<move_only> v{ 
				rref(move_only()), 
				rref(move_only()), 
				rref(move_only()) 
			};

		Or you can:
			using move_only = std::unique_ptr<int>;
			move_only init[] = { move_only(), move_only(), move_only() };
			std::vector<move_only> v{std::make_move_iterator(std::begin(init)),
				std::make_move_iterator(std::end(init))};
		*/

		template <typename T>
		class pointer_wrapper {
		public:
			using type = T;
			using pointer_type = observer_ptr<T>;

			pointer_wrapper(T& lvalue) noexcept : ptr_(std::addressof(lvalue)) {}
			pointer_wrapper(T&&) = delete;
			pointer_wrapper(const pointer_wrapper&) noexcept = default;
			pointer_wrapper& operator=(const pointer_wrapper& pw) noexcept = default;

			operator pointer_type& () const noexcept { return ptr_; }
			pointer_type& get() const noexcept { return ptr_; }
			pointer_type& operator->() { return ptr_; }
			type& operator*() { return *ptr_; }

			template <typename ...Args>
			typename std::result_of<T&(Args...)>::type operator() (Args&... args) const {
				return std::invoke(*get(), std::forward<Args...>(args)...);
			}

		private:
			observer_ptr<T> ptr_;
		};

		template <typename T>
		pointer_wrapper<T> make_ptr_wrapper(T&& t) {
			return pointer_wrapper{ std::forward<T>(t) };
		}

	}

    namespace recur_wrapper {
		template <typename T>
		class unique_wrapper {
			std::unique_ptr<T> m_storage;
		public:
			template <typename ...Args>
			unique_wrapper(Args&&... args)
				: m_storage(std::make_unique<T>(std::forward<Args>(args)...)) {}
			template <typename U>
			operator U& () noexcept {
				return static_cast<U&>(*m_storage);
			}
			template <typename U>
			operator const U& () noexcept {
				return static_cast<const U&>(*m_storage);
			}
			void swap(unique_wrapper& rhs) noexcept {
				m_storage.swap(rhs.m_storage);
			}
		};

		template <typename T>
		struct unwrap_type {};

		template <typename T>
		struct unwrap_type<unique_wrapper<T>>
			: std::decay<T> {};
		
		template <typename T>
		struct unref_type {};

		template <typename T>
		struct unref_type<unique_wrapper<T>>
			: std::remove_reference<T> {};

	}
}