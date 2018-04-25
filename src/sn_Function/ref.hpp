#ifndef SN_FUNCTION_REF_H
#define SN_FUNCTION_REF_H

namespace sn_Function {
    namespace ref {
        template <typename R, typename ...Args>
        class function_ref<R(Args...)> {
            void* ptr;
            R(*erased_fn)(void*, Args...);
        public:
            template <typename F> // some constraints here
            function_ref(F&& f) noexcept : ptr{&f} {
                erased_fn = [](void* ptr_, Args... xs) -> R {
                    return (*reintrepret_cast<F*>(ptr_))(
                        std::forward<Args>(xs)...
                    );
                };
            }
            // noexcept as call
            R operator()(Args... xs) const noexcept {
                return erased_fn(ptr, std::forward<Args>(xs)...);
            }
        };
    }
}



#endif