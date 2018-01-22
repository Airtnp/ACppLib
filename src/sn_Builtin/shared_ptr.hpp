#ifndef SN_BUILTIN_SHARED_PTR
#define SN_BUILTIN_SHARED_PTR

#include "../sn_CommonHeader.h"

namespace sn_Builtin {
    // ref: Effective Cpp
	// TODO: write real shared_ptr/weak_ptr
	// TODO: fix enable_shared_from_this multiple inheritance problem
	// ref: https://zhuanlan.zhihu.com/p/25065603
	// ref: https://www.codeproject.com/articles/286304/solution-for-multiple-enable-shared-from-this-in-i
	namespace shared_ptr {

#ifdef SN_ENABLE_TEMPORARY_UNAVAILABLE
		class enable_shared_from_this;

		struct CtrlCountBase {
			std::atomic<uint64_t> cnt;
			CtrlCountBase() : cnt{1} {}
			virtual void destroy(U*) = 0;
			virtual ~CtrlCountBase() {}
		};

		template <typename U>
		struct DefaultDeleter {
			void operator()(U* p) const {
				delete p;
			}
		};

		template <typename Deleter = DefaultDeleter>
		struct CtrlCount : CtrlCountBase {
			Deleter d;
			CtrlCout(Deleter d_) : d{d_} {}
			virtual void destroy(U* p) { d(p); }
		};

		template <typename T>
		struct CtrlBlkBase {
			explicit CtrlBlkBase(T* t) : m_t(t) {}

			// /* type-erased deleter */ deleter;
			// size_t shared_ref_count_;
			// size_t weak_ref_count_;
			T *m_t;
		};

		template <typename T, bool MakeShared>
		struct CtrlBlk;

		template <typename T>
		struct CtrlBlk<T, /* MakeShared = */ true> {
			template <typename... Args>
			explicit CtrlBlk(Args &&... args)
				: CtrlBlkBase<T>(nullptr), m_in_place(std::forward<Args>(args)...) {
				this->m_t = &m_in_place;
			}
			CtrlCount m_cnt;
			T m_in_place;
		};

		template <typename T>
		struct CtrlBlk<T, /* MakeShared = */ false> : CtrlBlkBase<T> {
			using CtrlBlkBase<T>::CtrlBlkBase;
			CtrlBlk(T* ptr) : CtrlBlkBase<T>{ptr} {}
			CtrlCount m_cnt;
		};

		template <typename T>
		struct shared_ptr {
			shared_ptr(T* t) : shared_ptr(new CtrlBlk<T, false>(t)) {}
			~shared_ptr() {
				dec_counter();
			}
			shared_ptr(const shared_ptr& other) : m_ctrl_blk{other.m_ctrl_blk} { inc_counter(); }
			
			shared_ptr& operator=(const shared_ptr& other) { 
				if (this != &other) {
					dec_counter();
					m_ctrl_blk = other.m_ctrl_blk;
					inc_counter();
				}
				return *this;
			}
			
			T* get() { 
				return m_ctrl_blk->m_t;
			}

			T* operator->() {
				return m_ctrl_blk->m_t;
			}

			T& operator*() {
				return *m_ctrl_blk->m_t;
			}

		private:
			// prevent extra copy / performance problem
			shared_ptr(CtrlBlk<T, true> *cb) : m_ctrl_blk(cb) {}
			shared_ptr(CtrlBlk<T, false>* cb) : m_ctrl_blk(cb) {}

			void inc_counter() {
				if (m_ctrl_blk->m_t) {
					m_ctrl_blk->m_cnt.fetch_add(1);
				}
			}

			void dec_counter() {
				if (m_ctrl_blk->m_t && m_ctrl_blk->m_cnt.fetch_sub(1) == 1) {
					m_ctrl_blk->m_cnt.destroy(m_ctrl_blk->m_t);
				}
			}

			CtrlBlkBase<T>* m_ctrl_blk;
			template <typename U, typename ...Args>
			friend shared_ptr<U> make_shared(Args&&... args);
		};

		template <typename T, typename ...Args>
		shared_ptr<T> make_shared(Args&&... args) {
			return shared_ptr<T>(new CtrlBlk<T, true>(std::forward<Args>(args)...));
		}

		// Is inheritent from this, shared_ptr ctor set the m_ptr = *this;
		template <typename T>
		class enable_shared_from_this {
		protected:
			constexpr enable_shared_from_this() {}
			enable_shared_from_this(const enable_shared_from_this&) {}
			enable_shared_from_this& operator=(const enable_shared_from_this&) {
				return *this;
			}
			~enable_shared_from_this() {}
		public:
			shared_ptr<T> shared_from_this() {
				return m_ptr.lock();
			}
			const shared_ptr<T> shared_from_this() const {
				return m_ptr.lock();
			}
		private:
			mutable weak_ptr<T> m_ptr;
			friend shared_ptr<T>;
		};
#endif
	}
}







#endif