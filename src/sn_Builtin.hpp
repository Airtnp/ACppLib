#ifndef SN_BUILTIN_H
#define SN_BUILTIN_H

#include "sn_CommonHeader.h"
#include "sn_Builtin/wrapper.hpp"
#include "sn_Builtin/observer_ptr.hpp"
#include "sn_Builtin/shared_ptr.hpp"
#include "sn_Builtin/intrusive_ptr.hpp"



namespace sn_Builtin {
	
	template <typename T>
	void safe_delete(T* p) {
		using complete_type_check = T;
		sizeof(complete_type_check);
		delete p;
	}

}

#endif