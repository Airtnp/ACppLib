#ifndef SN_STDSTREAM_H
#define SN_STDSTREAM_H

#include "sn_CommonHeader.h"

namespace sn_StdStream {
	namespace manipulator {
		template <class CharT, class Traits>
		std::basic_ostream<CharT, Traits>& nl(std::basic_ostream<CharT, Traits>& os) {
			os.puts('\n');
			return os;
		}
	}
}

namespace std {
	using sn_StdStream::manipulator::nl;
}

#endif