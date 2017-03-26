#ifndef SN_TEST_THREAD_H
#define SN_TEST_THREAD_H

#include "sn_CommonHeader_test.h"

namespace sn_Thread_test {
	void sn_thread_test() {
		observer_ptr<int> p = new int[100];
		const auto sg = sn_Thread::scope_guard::make_scope([&p] {
			delete[] p;
		});
		using p_t = sn_Thread::lock_guard::IsLockableAndUnLockable<std::mutex>;
		const auto has_lock = p_t::value;
	}
}







#endif