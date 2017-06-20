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
		struct CtrlBlk<T, /* MakeShared = */ true> : CtrlBlkBase<T> {
			template <typename... Args>
			explicit CtrlBlk(Args &&... args)
				: CtrlBlkBase<T>(nullptr), m_in_place(std::forward<Args>(args)...) {
				this->m_t = &m_in_place;
			}
			T m_in_place;
		};

		template <typename T>
		struct CtrlBlk<T, /* MakeShared = */ false> : CtrlBlkBase<T> {
			using CtrlBlkBase<T>::CtrlBlkBase;
		};

		template <typename T>
		struct shared_ptr {
			shared_ptr(T* t) : shared_ptr(new CtrlBlk<T, false>(t)) {}
			// shared_ptr(const weak_ptr<T>& rhs) : shared_ptr(new CtrlBlk<T, false>(rhs.lock())) {}
			T* get() const { 
				return m_t; 
			}
		private:
			// prevent extra copy / performance problem
			shared_ptr(CtrlBlk<T, true> *cb) : m_ctrl_blk(cb), m_t(m_ctrl_blk->m_t) {}
			CtrlBlkBase<T>* m_ctrl_blk;
			T* m_t;

			template <typename U, typename ...Args>
			friend shared_ptr<U> make_shared(Args&&... args);
		};

		template <typename T, typename ...Args>
		shared_ptr<T> make_shared(Args&&... args) {
			return shared_ptr<T>(new CtrlBlk<T, true>(std::forward<Args>(args)...));
		}

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