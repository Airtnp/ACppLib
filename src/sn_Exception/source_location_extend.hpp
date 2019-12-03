//
// Created by Airtnp on 10/18/2019.
//

#ifndef ACPPLIB_SOURCE_LOCATION_EXTEND_HPP
#define ACPPLIB_SOURCE_LOCATION_EXTEND_HPP

#include <utility>

namespace sn_Exception {
    namespace source_location {
        /// For `std::source_location` after variadic template argumentss
        template <typename ...Ts>
        struct vdebug {
            vdebug(Ts&&... ts, const std::source_location& loc = std::source_location::current());
        };
        template <typename ...Ts>
        vdebug(Ts&&...) -> vdebug<Ts...>;
    }
}



#endif //ACPPLIB_SOURCE_LOCATION_EXTEND_HPP
