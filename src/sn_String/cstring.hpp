#ifndef SN_STRING_CSTRING_H
#define SN_STRING_CSTRING_H

#include "../sn_CommonHeader.h"

namespace sn_String {
    namespace cstring {
		class CStringHelper {
		public: //unsigned  = unsigned int
			CStringHelper(const char* str, unsigned int len) : str_(str), len_(len) {
				assert(strlen(str_) == len_);
			}
			const char* str_;
			const unsigned int len_;
		};

		template <typename CharT>
		using basic_zstring = CharT*;
		using zstring = basic_zstring<char>;
		using czstring = basic_zstring<const char>;
		using wzstring = basic_zstring<wchar_t>;
		using cwzstring = basic_zstring<const wchar_t>;

	}
}



#endif