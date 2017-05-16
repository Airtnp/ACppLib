#ifndef SN_EXCEPTION_H
#define SN_EXCEPTION_H

#include "sn_CommonHeader.h"
#include "sn_Macro.hpp"

namespace sn_Exception {

    template <typename F>
    class ScopeGuard {
    public:
        ScopeGuard(const F& f)
            : m_func(f) {}
        ScopeGuard(F&& f)
            : m_func(std::move(f)) {}
        ~ScopeGuard() noexcept(true) {
            m_func();
        }
    private:
        F   m_func;
    };

    // C++1z: int uncaught_exceptions()
    // Detects how many exceptions in the current thread 
    // have been thrown or rethrown 
    // and not yet entered their matching catch clauses.
    class UncaughtExceptionCounter {
        int getUncaughtExceptionCount() const noexcept {
            return m_exceptionCount;
        }
        int m_exceptionCount;
    public:
        UncaughtExceptionCounter()
            : m_exceptionCount(std::uncaught_exceptions()) {}
        bool HasNewUncaughtException() noexcept {
            return std::uncaught_exceptions() > m_exceptionCount;
        }
    }

    template <typename F, bool execOnException>
    class ScopeGuardForException {
        F m_func;
        UncaughtExceptionCounter m_ec;
    public:
        explicit ScopeGuardForException(const F& fn)
            : m_func(fn) {}
        explicit ScopeGuardForException(F&& fn)
            : m_func(std::move(fn)) {}
        ~ScopeGuardForException() noexcept(execOnException) {
            if (execOnException == m_ec.HasNewUncaughtException())
                m_func();
        }
    }

    enum class ScopeGuardOnFail {};
    enum class ScopeGuardOnExit {};
    enum class ScopeGuardOnSuccess {};
    
    template <typename F>
    ScopeGuard<std::decay_t<F>> operator+(ScopeGuardOnExit, F&& fn) {
        return ScopeGuard<std::decay_t<F>>(std::forward<F>(fn));
    }

    template <typename F>
    ScopeGuardForException<std::decay_t<F>, true> operator+(ScopeGuardOnFail, F&& fn) {
        return ScopeGuardForException<std::decay_t<F>, true>(std::forward<F>(fn));
    }

    template <typename F>
    ScopeGuardForException<std::decay_t<F>, false> operator+(ScopeGuardOnSuccess, F&& fn) {
        return ScopeGuardForException<std::decay_t<F>, false>(std::forward<F>(fn));
    }

#define SCOPE_EXIT \
    auto ANONYMOUS_VARIABLE(SCOPE_EXIT_STATE) \
    = sn_Exception::ScopeGuardOnExit() + [&]()

#define SCOPE_FAIL \
    auto ANONYMOUS_VARIABLE(SCOPE_FAIL_STATE) \
    = sn_Exception::ScopeGuardOnFailure() + [&]()

#define SCOPE_SUCCESS \
    auto ANONYMOUS_VARIABLE(SCOPE_SUCCESS_STATE) \
    = sn_Exception::ScopeGuardOnSuccess() + [&]()

}




#endif