#ifndef SN_STREAM_H
#define SN_STREAM_H

#include "sn_CommonHeader.h"
#include "sn_Builtin.hpp"


namespace sn_Stream {
	namespace stream {
		enum class seek {
			Begin, Current, End,
		};
		class Stream : public sn_Builtin::reference_counter::IReferenceCounter {

		};
	}
}






#endif