#ifndef SN_CONFIG_ENDIAN_H
#define SN_CONFIG_ENDIAN_H

namespace sn_Config {
    namespace endian {

        // or __BIG_ENDIAN__ ...
        enum endian : uint32_t {
            SN_LITTLE_ENDIAN   = 0x00000001,
            SN_BIG_ENDIAN      = 0x01000000,
            SN_PDP_ENDIAN      = 0x00010000,
            SN_UNKNOWN_ENDIAN  = 0xFFFFFFFF
        };

        constexpr endian get_endian() {
            return
                ((1 & 0xFFFFFFFF) == SN_LITTLE_ENDIAN)
                    ? SN_LITTLE_ENDIAN
                    : ((1 & 0xFFFFFFFF) == SN_BIG_ENDIAN)
                        ? SN_BIG_ENDIAN
                        : ((1 & 0xFFFFFFFF) == SN_PDP_ENDIAN)
                            ? SN_PDP_ENDIAN
                            : SN_UNKNOWN_ENDIAN;
        }
    }
} 


#endif