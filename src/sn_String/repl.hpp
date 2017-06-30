#ifndef SN_STRING_REPL_H
#define SN_STRING_REPL_H

#include "../sn_CommonHeader.h"

namespace sn_String {
    namespace repl {
        using std::to_string;
        std::string to_string(std::string input) {
            return input;
        }

        std::string to_string(char* input) {
            return static_cast<std::string>(input);
        }
    }
}



#endif