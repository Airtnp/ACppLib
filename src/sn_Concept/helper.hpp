//
// Created by xiaol on 11/29/2019.
//

#ifndef ACPPLIB_HELPER_HPP
#define ACPPLIB_HELPER_HPP

#include "sn_Macro.hpp"

namespace sn_Concept {

#define SN_CONCEPT_GEN_PARAM_IMPL(I) \
    T##I

#define SN_CONCEPT_GEN_PARAM(N) \
    SN_APPLY_MULTI_WITH_SEP_N(N, (,), SN_CONCEPT_GEN_PARAM_IMPL)

#define SN_CONCEPT_GEN_TEMP_IMPL(I) \
    typename T##I

#define SN_CONCEPT_GEN_TEMP(N) \
    SN_APPLY_MULTI_WITH_SEP_N(N, (,), SN_CONCEPT_GEN_TEMP_IMPL)

#define SN_CONCEPT_CAST(NAME, CONCEPT, N) \
    template < SN_CONCEPT_GEN_TEMP(N) > \
    struct NAME : std::conditional_t< CONCEPT < SN_CONCEPT_GEN_PARAM(N) > , std::true_type, std::false_type> {}; \
    \
    template < SN_CONCEPT_GEN_TEMP(N) > \
    inline constexpr bool MACRO_CONCAT(NAME, v) = NAME< SN_CONCEPT_GEN_PARAM(N) >::value;

}

#endif //ACPPLIB_HELPER_HPP
