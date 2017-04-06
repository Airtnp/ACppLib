#ifndef SN_RANGE_H
#define SN_RANGE_H

#include "sn_CommonHeader.h"

namespace sn_Range {
	namespace apply {

	}

	namespace for_range{
		template <std::size_t N>
		struct StaticForRange {};

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
		FakeIt begin(StaticForRange<N>& fr) {
			return { 0 };
		}

		template <std::size_t N>
		FakeIt end(StaticForRange<N>& fr) {
			return { N };
		}

		template <std::size_t N>
		using SFR = StaticForRange<N>;

		struct ForRange {
			const std::size_t m_n;
			ForRange(std::size_t n) : m_n(n) {}
			std::size_t end() const noexcept {
				return m_n;
			}
		};

		FakeIt begin(ForRange& fr) {
			return{ 0 };
		}

		FakeIt end(ForRange& fr) {
			return{ fr.end() };
		}

	}

}








#endif