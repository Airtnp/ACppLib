#include "../sn_CommonHeader.h"
#include "../sn_Assist.hpp"

namespace sn_Builtin {
    //ref: natsulib/natrefobj boost/intrusive_ptr
	namespace reference_counter {
		template <typename T>
		class IntrusiveReferencePtr : sn_Assist::sn_functional_base::less_than<IntrusiveReferencePtr<T>> {
		private:
			using this_type = IntrusiveReferencePtr<T>;
		public:
			using element_type = T;
			using pointer_type = std::add_pointer_t<T>;
			constexpr IntrusiveReferencePtr(std::nullptr_t = nullptr) noexcept : ptr_(nullptr) {}
			constexpr explicit IntrusiveReferencePtr(observer_ptr<T> p, bool add_ref = true) : ptr_(p) {}
			~IntrusiveReferencePtr() {
				if (ptr_)
					release();
			}
			constexpr IntrusiveReferencePtr(const IntrusiveReferencePtr& rhs) : ptr_(rhs.ptr_) {
				if (ptr_)
					add_ref();
			}
			template <typename U>
			IntrusiveReferencePtr(const IntrusiveReferencePtr<U>& rhs) : ptr_(rhs.get()) {
				if (ptr_)
					add_ref();
			}
			IntrusiveReferencePtr& operator=(std::nullptr_t) {
				if (ptr_)
					release();
				return *this;
			}
			IntrusiveReferencePtr& operator=(const IntrusiveReferencePtr& rhs) {
				IntrusiveReferencePtr(rhs).swap(*this);
				return *this;
			}
			template <typename U>
			IntrusiveReferencePtr& operator=(const IntrusiveReferencePtr<U>& rhs) {
				IntrusiveReferencePtr(rhs).swap(*this);
				return *this;
			}

			constexpr IntrusiveReferencePtr(IntrusiveReferencePtr&& rhs) noexcept : ptr_(rhs.ptr_) {
				rhs.ptr_ = nullptr;
			}
			IntrusiveReferencePtr& operator=(IntrusiveReferencePtr&& rhs) noexcept {
				IntrusiveReferencePtr(static_cast<IntrusiveReferencePtr&&>(rhs)).swap(*this);
				return *this;
			}

			template <typename U>
			friend class IntrusiveReferencePtr;
			
			template <typename U>
			IntrusiveReferencePtr(IntrusiveReferencePtr<U>&& rhs) noexcept : ptr_(rhs.ptr_) {
				rhs.ptr_ = nullptr;
			}
			template <typename U>
			IntrusiveReferencePtr& operator=(IntrusiveReferencePtr<U>&& rhs) noexcept {
				IntrusiveReferencePtr(static_cast<IntrusiveReferencePtr<U>&&>(rhs)).swap(*this);
				return *this;
			}

			void reset() noexcept {
				IntrusiveReferencePtr().swap(*this);
			}

			void reset(observer_ptr<T> rhs) {
				IntrusiveReferencePtr(rhs).swap(*this);
			}

			void reset(observer_ptr<T> rhs, bool add_ref) {
				IntrusiveReferencePtr(rhs, add_ref).swap(*this);
			}

			observer_ptr<T> get() const {
				return ptr_;
			}

			observer_ptr<T> detach() const {
				observer_ptr<T> ret = ptr_;
				ptr_ = nullptr;
				return ret;
			}

			operator observer_ptr<T>() const {
				return ptr_;
			}

			T& operator*() const {
				assert(ptr_);
				return *ptr_;
			}

			observer_ptr<T> operator->() const {
				assert(ptr_);
				return ptr_;
			}

			explicit operator bool() const noexcept {
				return ptr_ != nullptr;
			}
			
			//add concept CanAddReference
			virtual void add_ref() {
				//static_cast<observer_ptr<T>>(this)->add_ref();
				ptr_->add_ref();
			}

			/*
			virtual void sub_ref() {
				static_cast<observer_ptr<T>>(this)->sub_ref();
			}
			*/

			//add concept CanRelease
			virtual void release() {
				//static_cast<observer_ptr<T>>(this)->release();
				ptr_->release();
			}

			virtual void get_ref_count() {
				//static_cast<observer_ptr<T>>(this)->get_ref_count();
				ptr_->get_ref_count();
			}

			void swap(IntrusiveReferencePtr& rhs) noexcept {
				observer_ptr<T> tmp = ptr_;
				ptr_ = rhs.ptr_;
				rhs.ptr_ = tmp;
			}

			template <typename U>
			bool operator==(const IntrusiveReferencePtr<U>& rhs) const noexcept {
				return ptr_ == rhs.get();
			}

			template <typename U>
			bool operator<(const IntrusiveReferencePtr<U>& rhs) const noexcept {
				return ptr_ < rhs.get();
			}


		private:
			observer_ptr<T> ptr_;
		};

		template <typename T>
		using IR_ptr = IntrusiveReferencePtr<T>;

		template <typename T, typename ...Args>
		inline IR_ptr<T> make_ref_ptr(Args&&... args) {
			auto ref_obj = new T(std::forward<Args>(args)...);
			IR_ptr<T> ret(ref_obj);
			ref_obj->release();
			return std::move(ret);
		}


		class IReferenceCounter {
		public:
			virtual ~IReferenceCounter() = default;

			virtual std::size_t get_ref_count() const volatile noexcept = 0;
			virtual bool try_add_ref() const volatile = 0;
			virtual void add_ref() const volatile = 0;
			virtual bool release() const volatile = 0;
		};

		template <typename Base = IReferenceCounter>
		class ReferenceCounterBase : public Base {
		public:
			bool is_unique() const volatile noexcept {
				return get_ref_count() == 1;
			}

			virtual std::size_t get_ref_count() const volatile noexcept {
				return m_refCount.load(std::memory_order_relaxed);
			}

			virtual bool try_add_ref() const volatile {
				auto old_value = m_refCount.load(std::memory_order_relaxed);
				assert(static_cast<std::ptrdiff_t>(old_value) >= 0);
				do {
					if (!old_value)
						return false;
				} while (!m_refCount.compare_exchange_strong(old_value, old_value + 1, std::memory_order_relaxed, std::memory_order_relaxed));
				return true;
			}

			virtual void add_ref() const volatile {
				assert(static_cast<std::ptrdiff_t>(m_refCount.load(std::memory_order_relaxed)) > 0);
				m_refCount.fetch_add(1, std::memory_order_relaxed);
			}

			virtual bool release() const volatile {
				assert(static_cast<std::ptrdiff_t>(m_refCount.load(std::memory_order_relaxed)) > 0);
				return m_refCount.fetch_sub(1, std::memory_order_relaxed) == 1;
			}

		protected:
			constexpr ReferenceCounterBase() noexcept : m_refCount(1) {}
			constexpr ReferenceCounterBase(const ReferenceCounterBase&) noexcept : ReferenceCounterBase() {}
			ReferenceCounterBase& operator=(const ReferenceCounterBase&) noexcept {
				return *this;
			}
			constexpr ReferenceCounterBase(const ReferenceCounterBase&&) noexcept = delete;
			ReferenceCounterBase& operator=(const ReferenceCounterBase&&) noexcept = delete;
			~ReferenceCounterBase() {
				assert(ReferenceCounterBase::get_ref_count() <= 1);
			}

		private:
			mutable std::atomic<std::size_t> m_refCount;

		};

		template <typename T>
		class WeakReferenceView final : public ReferenceCounterBase<IReferenceCounter> {
		public:
			using owner_type = T;
			using owner_ptr_t = std::add_pointer_t<T>;

			constexpr explicit WeakReferenceView(owner_ptr_t owner) noexcept : m_mutex{}, m_owner{ owner } {}
			bool is_owner_alive() const {
				const std::lock_guard<std::mutex> lock{ m_mutex };
				const auto owner = m_owner;
				return owner && static_cast<const volatile observer_ptr<ReferenceCounterBase>>(owner)->get_ref_count() > 0;
			}

			bool clear_owner() {
				const std::lock_guard<std::mutex> lock{ m_mutex };
				m_owner = nullptr;
				return true;
			}

			template <typename U>
			IR_ptr<U> lock_owner() const {
				const std::lock_guard<std::mutex> lock{ m_mutex };
				const auto other = sn_Assist::sn_cast::static_dynamic_cast<owner_ptr_t>(m_owner);
				if (!other)
					return {};
				if (!static_cast<std::add_cv_t<decltype(m_owner)>>(m_owner)->try_add_ref())
					return {};
				return{ other };
			}

		private:
			mutable std::mutex m_mutex;
			owner_ptr_t m_owner;
		};

		template <typename T>
		class IntrusiveWeakReferencePtr : public sn_Assist::sn_functional_base::less_than<IntrusiveWeakReferencePtr<T>> {
		private:
			template <typename U>
			friend class IntrusiveWeakReferencePtr;

			using t_WRV = typename T::t_WRV;
			using p_WRV = observer_ptr<t_WRV>;

			static p_WRV get_view_from(const volatile observer_ptr<T> item) {
				if (!item)
					return nullptr;
				const auto view = item->create_weak_ref_view();
				view->add_ref();
				return view;
			}

		public:
			using pointer_type = std::add_pointer_t<T>;

			constexpr IntrusiveWeakReferencePtr(std::nullptr_t = nullptr) noexcept : m_view{} {}
			constexpr IntrusiveWeakReferencePtr(pointer_type ptr) noexcept : IntrusiveWeakReferencePtr(get_view_from(ptr)) {}

			template <typename U, typename std::enable_if_t<std::is_convertible<typename IntrusiveWeakReferencePtr<U>::pointer_type, pointer_type>::value, int> = 0>
			IntrusiveWeakReferencePtr(const IntrusiveWeakReferencePtr<U>& rhs) noexcept : IntrusiveWeakReferencePtr(get_view_from(rhs.fork())) {}

			template <typename U, typename std::enable_if_t<std::is_convertible<typename IntrusiveWeakReferencePtr<U>::pointer_type, pointer_type>::value, int> = 0>
			IntrusiveWeakReferencePtr(IntrusiveWeakReferencePtr<U>&& rhs) noexcept : IntrusiveWeakReferencePtr(rhs.release()) {}

			template <typename U, typename std::enable_if_t<std::is_convertible<typename IntrusiveWeakReferencePtr<U>::pointer_type, pointer_type>::value, int> = 0>
			IntrusiveWeakReferencePtr(const IR_ptr<U>& rhs) noexcept : IntrusiveWeakReferencePtr(get_view_from(rhs.get())) {}

			IntrusiveWeakReferencePtr(const IntrusiveWeakReferencePtr& rhs) noexcept : IntrusiveWeakReferencePtr(get_view_from(rhs.fork())) {}

			IntrusiveWeakReferencePtr(IntrusiveWeakReferencePtr&& rhs) noexcept : IntrusiveWeakReferencePtr(rhs.release()) {}

			IntrusiveWeakReferencePtr& operator=(const IntrusiveWeakReferencePtr& rhs) noexcept {
				reset(rhs);
				return *this;
			}

			IntrusiveWeakReferencePtr& operator=(IntrusiveWeakReferencePtr&& rhs) noexcept {
				reset(std::move(rhs));
				return *this;
			}


			~IntrusiveWeakReferencePtr() {
				if (m_view)
					view_release();
			}

			bool is_expired() const noexcept {
				const auto view = m_view;
				if (!view)
					return true;
				return !view->is_owner_alive();
			}

			std::size_t weak_count() const noexcept {
				const auto view = m_view;
				if (!view)
					return 0;
				return view->get_ref_count() - 1;  //ptr has 1 ref to weakrefcount
			}

			template <typename U = T>
			IR_ptr<U> lock() const noexcept {
				const auto view = m_view;
				if (!view)
					return {};
				return view->template lock_owner<U>();
			}

			void reset(pointer_type ptr) noexcept {
				IntrusiveWeakReferencePtr{ ptr }.swap(*this);
			}

			template <typename U>
			void reset(const IR_ptr<U>& ptr) noexcept {
				IntrusiveWeakReferencePtr{ ptr }.swap(*this);
			}

			template <typename U>
			void reset(const IntrusiveWeakReferencePtr<U>& ptr) noexcept {
				IntrusiveWeakReferencePtr{ ptr }.swap(*this);
			}

			template <typename U>
			void reset(const IntrusiveWeakReferencePtr<U>&& ptr) noexcept {
				IntrusiveWeakReferencePtr{ std::move(ptr) }.swap(*this);
			}

			void reset(const IntrusiveWeakReferencePtr& ptr) noexcept {
				IntrusiveWeakReferencePtr{ ptr }.swap(*this);
			}

			void reset(const IntrusiveWeakReferencePtr&& ptr) noexcept {
				IntrusiveWeakReferencePtr{ std::move(ptr) }.swap(*this);
			}

			void swap(IntrusiveWeakReferencePtr& other) noexcept {
				const auto view = m_view;
				m_view = other.m_view;
				other.m_view = view;
			}

			template <typename U>
			bool operator==(const IntrusiveWeakReferencePtr& rhs) const noexcept {
				return m_view == rhs.m_view;
			}

			template <typename U>
			bool operator<(const IntrusiveWeakReferencePtr& rhs) const noexcept {
				return m_view < rhs.m_view;
			}


			void view_release() {
				m_view->release();
			}


		private:
			p_WRV m_view;

			constexpr explicit IntrusiveWeakReferencePtr(p_WRV view) noexcept : m_view{ view } {}

			p_WRV fork() const {
				const auto view = m_view;
				if (view)
					static_cast<const volatile observer_ptr<IReferenceCounter>>(m_view)->add_ref();
				return view;
			}

			p_WRV release() noexcept {
				return std::exchange(m_view, nullptr);
			}

		};

		template <typename T>
		using IWR_ptr = IntrusiveWeakReferencePtr<T>;

		template <typename T = IReferenceCounter, typename Deleter = std::default_delete<T>>
		class ReferenceCounter : public ReferenceCounterBase<T> {
			static_assert(std::is_base_of<IReferenceCounter, T>::value, "T should inherit from IReferenceCounter");
		private:
			template <typename U>
			friend class IntrusiveWeakReferencePtr;
			

			template <typename U, typename ...Args>
			friend inline IR_ptr<U> make_ref_ptr(Args&& ...args);

		protected:
			// Add in-place/nothrow version
			void* operator new(std::size_t size) {
				return ::operator new(size);
			}

			void operator delete(void* ptr) {
				::operator delete(ptr);
			}

		public:

			using Base = ReferenceCounterBase<T>;
			using DefaultBase = ReferenceCounterBase<IReferenceCounter>;
			using deleter_type = Deleter;
			using t_WRV = WeakReferenceView<ReferenceCounter>;
			using p_WRV = std::add_pointer_t<t_WRV>;

			constexpr ReferenceCounter() noexcept : m_view{nullptr} {}
			constexpr ReferenceCounter(const ReferenceCounter&) noexcept : m_view{ nullptr } {}
			ReferenceCounter& operator=(const ReferenceCounter&) noexcept {
				return *this;
			}

			virtual ~ReferenceCounter() {
				const auto view = m_view.load(std::memory_order_consume);
				if (view) {
					if (static_cast<const observer_ptr<DefaultBase>>(view)->release()) {
						delete view;
					}
					else {
						view->clear_owner();
					}
				}
			}

			bool release() const volatile override {
				const auto res = Base::release();
				if (res)
					deleter_type{}(const_cast<observer_ptr<ReferenceCounter>>(this));
				return res;
			}

			template <typename U>
			IR_ptr<U> fork_ref() noexcept {
				return fork_ref_impl<U>(this);
			}

			template <typename U>
			IR_ptr<const U> fork_ref() const noexcept {
				return fork_ref_impl<const U>(this);
			}

			template <typename U>
			IR_ptr<volatile U> fork_ref() volatile noexcept {
				return fork_ref_impl<volatile U>(this);
			}

			template <typename U>
			IR_ptr<const volatile U> fork_ref() const volatile noexcept {
				return fork_ref_impl<const volatile U>(this);
			}

			template <typename U>
			IWR_ptr<U> fork_weak_ref() noexcept {
				return fork_weak_ref_impl<U>(this);
			}

			template <typename U>
			IWR_ptr<const U> fork_weak_ref() const noexcept {
				return fork_weak_ref_impl<const U>(this);
			}

			template <typename U>
			IWR_ptr<volatile U> fork_weak_ref() volatile noexcept {
				return fork_weak_ref_impl<volatile U>(this);
			}

			template <typename U>
			IWR_ptr<const volatile U> fork_weak_ref() const volatile noexcept {
				return fork_weak_ref_impl<const volatile U>(this);
			}

		private:

			template <typename U, typename W>
			IR_ptr<U> fork_ref_impl(observer_ptr<W> p) {
				const auto other = sn_Assist::sn_cast::static_dynamic_cast<observer_ptr<U>>(p);
				if (!p)
					return {};
				return IR_ptr<U>{other};
			}

			template <typename U, typename W>
			IWR_ptr<U> fork_weak_ref_impl(observer_ptr<W> p) {
				const auto other = sn_Assist::sn_cast::static_dynamic_cast<observer_ptr<U>>(p);
				if (!p)
					return{};
				return IWR_ptr<U>{other};
			}

			p_WRV create_weak_ref_view() const volatile {
				auto view = m_view.load(std::memory_order_consume);
				if (!view) {
					const auto n_view = new t_WRV(const_cast<observer_ptr<ReferenceCounter>>(this));
					if (m_view.compare_exchange_strong(view, n_view, std::memory_order_release, std::memory_order_consume))
						view = n_view;
					else
						delete n_view;
				}
				return view;
			}

			mutable std::atomic<p_WRV> m_view;

		};

		//IWR_ptr -> WRV_ptr -> IR_ptr

	}

	
}