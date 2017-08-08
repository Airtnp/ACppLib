#ifndef SN_CONFIG_OPTIMIZE_H
#define SN_CONFIG_OPTIMIZE_H

namespace sn_Config {
    namespace optimize {
        struct debugT {};
        struct releaseT {};
    }
}

#if defined(_DEBUG) || defined (NDEBUG)

typedef debugT compiler_optimization_option;
#define SN_CONFIG_OPTIMIZATION_DEBUG
#define SN_CONFIG_OPTIMIZATION_REPL "debug"

#else

typedef debugT compiler_optimization_option;
#define SN_CONFIG_OPTIMIZATION_DEBUG
#define SN_CONFIG_OPTIMIZATION_REPL "debug"

#endif


#endif