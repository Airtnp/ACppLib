#ifndef SN_CONFIG_OPTIMIZE_H
#define SN_CONFIG_OPTIMIZE_H

namespace sn_Config {

    namespace optimize {
        struct debugT {};
        struct releaseT {};
    }
}

#if defined(_DEBUG) || defined (DEBUG)

typedef debugT compiler_optimization_option;
#define SN_CONFIG_OPTIMIZATION_DEBUG
#define SN_CONFIG_OPTIMIZATION_REPL "debug"

#else

typedef sn_Config::optimize::releaseT compiler_optimization_option;
#define SN_CONFIG_OPTIMIZATION_RELEASE
#define SN_CONFIG_OPTIMIZATION_REPL "release"

#endif


#endif