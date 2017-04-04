#ifndef SN_RANGE_H
#define SN_RANGE_H

#include "sn_CommonHeader.h"

namespace sn_Range {
	namespace apply {

	}

	namespace for_range{
		template <std::size_t N>
		struct ForRange {};

		struct FakeIt {
			std::size_t n;

			bool operator!=(const FakeIt& rhs) {
				return n != rhs.n;
			}

			FakeIt& operator++() {
				++n;
				return *this;
			}

			std::size_t operator*() {
				return n;
			}
		};


		template <std::size_t N>
		FakeIt begin(ForRange<N>& fr) {
			return { 0 };
		}

		template <std::size_t N>
		FakeIt end(ForRange<N>& fr) {
			return { N };
		}

		template <std::size_t N>
		using FR = ForRange<N>;

	}

}








#endif