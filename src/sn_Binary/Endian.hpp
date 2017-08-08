#ifndef SN_BINARY_ENDIAN_H
#define SN_BINARY_ENDIAN_H

#include "../sn_CommonHeader.h"

namespace sn_Binary {
    // ref: https://github.com/akemimadoka/NatsuLib
	// TODO: add constexpr judge ref: http://stackoverflow.com/questions/1001307/detecting-endianness-programmatically-in-a-c-program
	//							ref: https://codereview.stackexchange.com/questions/45675/checking-endianness-at-compile-time
	namespace Endian {
		enum class ByteEndian {
			BigEndian,
			LittleEndian,
			MiddleEndian, //Not supported (I am shocked to know it exists!)
		};

		ByteEndian getEndian() {
			static constexpr uint16_t s_short = 0x1234;
			static const auto s_endian = reinterpret_cast<const uint8_t*>(&s_short)[0] == 0x12 ? ByteEndian::BigEndian : ByteEndian::LittleEndian;
			return s_endian;
		}
		using len_t = uint64_t;
		using byte_t = uint8_t;
		using byteptr_t = observer_ptr<byte_t>;

		inline void swapEndian(byteptr_t data, std::size_t n) {
			using std::swap;
			const auto max = n - 1;
			const auto mid = n / 2;
			for (std::size_t i = 0; i < mid; ++i) {
				swap(data[i], data[max - i]);
			}
		}
	}
}


#endif