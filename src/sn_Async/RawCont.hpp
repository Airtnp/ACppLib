/// Taking from CppCon2019, A unifying abstraction for async

#ifndef SN_ASYNC_RAWCONT_H
#define SN_ASYNC_RAWCONT_H

#include "../sn_CommonHeader.h"

auto then(auto task, auto fun) {
    return [=](auto p) {
        struct _promise {
            decltype(p) p_;
            decltype(fun) fun_;
            void set_value(auto ...vs) {
                p_.set_value(fun_(vs...));
            }
            void set_exception(auto e) {
                p_.set_exception(e);
            }
        };
        task(_promise{p, fun});
    }
}

struct sink {
    void set_value(auto...) {}
    void set_exception(auto) {
        std::terminate();
    }
};


#endif