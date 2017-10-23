#ifndef SN_THREAD_THIS_THREAD_H
#define SN_THREAD_THIS_THREAD_H

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace sn_Thread {
    namespace this_thread {
#ifdef _WIN32
        HANDLE this_thread() {
            return ::GetCurrentHandle();
        }
#else
        pthread_t this_thread() {
            return ::pthread_self();
        }
#endif
    }
}

#endif