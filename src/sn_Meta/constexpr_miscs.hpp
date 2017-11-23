#ifndef SN_CONSTEXPR_MISCS_H
#define SN_CONSTEXPR_MISCS_H

namespace sn_Meta {

#define SN_CONSTEXPR_CONSTANT(...) \
    union { static constexpr auto value() { return __VA_ARGS__; } }

#define SN_CONSTEXPR_CONSTANT_VALUE(...) \
    [] { using R = SN_CONSTEXPR_CONSTANT(__VA_ARGS__); return R{}; }()
  
}


#endif