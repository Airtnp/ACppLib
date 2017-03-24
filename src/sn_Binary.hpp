#ifndef SN_BINARY_H
#define SN_BINARY_H

#include "sn_CommonHeader.h"

namespace sn_Binary {
	//ref: https://github.com/akemimadoka/NatsuLib
	namespace Endian {
		enum class Endian {
			BigEndian,
			LittleEndian,
			MiddleEndian, //Not supported (I am shocked to know it exists!)
		};

		Endian getEndian() {
			static constexpr uint16_t s_short = 0x1234;
			static const auto s_endian = reinterpret_cast<const uint8_t*>(&test_short) == 0x12 ? Endian::BigEndian : Endian::LittleEndian;
			return s_endian;
		}
	}

	namespace reader {
		
	}

}


#endif