#ifndef SN_THREAD_LEAF_H
#define SN_THREAD_LEAF_H

#include "../sn_CommonHeader.h"

// @ref: ACCU Zero Allocation
namespace sn_Thread {
    namespace Leaf {
        template <typename T>
        struct is_executable : std::false_type {};

        template <typename In, typename F>
        struct leaf : F {
            using in_type = In;
            using out_type = std::inovke_result<F&, In>;

            leaf(F&& f) : F{std::move(f)} {}
            template <typename Scheduler, typename Input, typename Then>
            void execute(Scheduler&, Input&& input, Then&& then) & {
                std::forward<Then>(then)(
                    ((*this)(std::forward<Input>(input)))
                );
            }

            template <typename X>
            auto then(X&& x) {
                if constexpr (is_executable<X>) {
                    return seq(std::move(*this), std::forward<X>(x));
                } else {
                    return seq(std::move(*this), leaf{std::forward<X>(x)});
                }
            }
        };

        using sn_Assist::function_traits;
        template <typename F>
        using first_arg_t = function_traits<F>::args<0>::type;

        template <typename R, typename Arg>
        leaf(R(*)(Arg)) -> leaf<Arg, R(*)(Arg)>;

        template <typename F>
        leaf(F&&) -> leaf<first_arg_t<F>, std::decay_t<F>>;

        template <typename T, typename U>
        struct seq : T, U {
            using in_type = typename T::in_type;
            using out_type = typename U::out_type;

            seq(T&& t, U&& u) : T{std::move(t)}, U{std::move(u)} {}
            template <typename Scheduler, typename Then>
            void execute(Scheduler& scheduler, Input&& input, Then&& then) & {
                static_cast<A&>(*this).execute(scheduler, 
                    std::forward<Input>(input), 
                    [this, &scheduler, then](auto&& r){
                        static_cast<B&>(*this).execute(scheduler, std::forward<decltype(r)>(r), then);
                });
            }
            template <typename X>
            auto then(X&& x) {
                if constexpr (is_executable<X>) {
                    return seq(std::move(*this), std::forward<X>(x));
                } else {
                    return seq(std::move(*this), leaf{std::forward<X>(x)});
                }
            }
        };

        template <typename T>
        struct aligned_storage_for {
            std::aligned_storage<sizeof(T), alignof(T)> data;
            template <typename ...Args>
            void construct(Args&&... args) {
                new (&data) T{std::forward<Args>(args)...};
            }
            T& access() noexcept {
                return reinterpret_cast<T&>(data);
            }
            const T& access() const noexcept {
                return reinterpret_cast<T&>(data);
            }
            void destroy() {
                access().~T();
            }
            T& operator*() noexcept {
                return access();
            }
            const T& operator*() const noexcept {
                return access();
            }
            T* operator->() noexcept {
                return &access();
            }
            const T* operator->() const noexcept {
                return &access();
            }
        };

        namespace detail {
            template <typename F, typename ...Args, auto ...Is>
            void enumerate_args_impl(std::index_sequence<Is...>, F&& f, Args&&... args) {
                (f(
                    std::integral_constant<decltype(Is), Is>,
                    std::forward<Args>(args)
                ), ...);
            }
        }

        template <typename T>
        struct type_wrapper {
            using type = T;
        };

        template <typename ...Args>
        void enumerate_args(F&& f) {
            detail::enumerate_args_impl(std::index_sequence_for<Args...>, std::forward<F>(f), type_warpper<Args>...);
        }

        template <typename ...Fs>
        struct all : Fs... {
            using in_type = std::common_type_t<typename Fs::in_type...>;
            using out_type = std::tuple<typename Fs::out_type ...>;

            struct shared_state {
                in_type input;
                std::atomic<int> left_cnt;

                template <typename Input>
                shared_state(Input&& input_) : input{std::forward<Input>(input_)} {
                    left_cnt.store(sizeof...(Fs), std::memory_order_release);
                }
            };

            aligned_storage_for<shared_state> state;
            out_type value;

            all(Fs&&... fs) : Fs{std::move(fs)}... {}
            template <typename Scheduler, typename Input, typename Then>
            void execute(Scheduler& sched, Input&& input, Then&& then) & {
                state.construct(std::forward<Input>(input));
                enumerate_args<Fs...>([&](auto i, auto t) {
                    sched([this, &sched, &f = static_cast<decltype(t)::type&>(*this), then]{
                        f.execute(sched, state->input, [this, then](auto&& r){
                            std::get<decltype(i){}>(value) = std::forward<decltype(r)>(r);
                            if (state->left_cnt.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                                state.destroy();
                                then(std::move(value));
                            }
                        });
                    });
                });
            }
        };

        template <typename T>
        class latch {
            std::condition_variable cv;
            std::mutex mtx;
            T ctr{};
        public:
            latch(T ctr_ = T{}) : ctr{ctr_} {}
            void count_down() {
                std::unique_lock lk{mtx};
                --ctr;
                cv.notify_all();
            }
            void wait() {
                std::unique_lock lk{mtx};
                cv.wait(lk, [this]{ return ctr == true; });
            }
        };

        template <typename ...Fs>
        struct any : Fs... {
            using in_type = std::common_type_t<typename Fs::in_type...>;
            using out_type = std::variant<typename Fs::out_type...>;

            struct shared_state {
                in_type input;
                std::atomic<int> left_cnt;

                template <typename Input>
                shared_state(Input&& input_) : input{std::forward<Input>(input_)} {
                    left_cnt.store(sizeof...(Fs), std::memory_order_release);
                }
            };

            aligned_storage_for<shared_state> state;
            out_type value;

            any(Fs&&... fs) : Fs{std::move(fs)}... {}

            template <typename Scheduler, typename Input, typename Then>
            void execute(Scheduler& sched, Input&& input, Then&& then) & {
                state.construct(std::forward<Input>(input));
                enumerate_args<Fs...>([&](auto i, auto t) {
                    sched([this, &sched, &f = static_cast<decltype(t)::type&>(*this), then]{
                        f.execute(sched, state->input, [this, then](auto&& r){
                            const auto res = state->left_cnt.fetch_sub(1, std::memory_order_acq_rel);
                            if (r == sizeof...(Fs)) {
                                value = std::forward<decltype(r)>(r);
                                then(std::move(value));
                            }
                            if (r == 1) {
                                state.destroy();
                            }
                        });
                    });
                });
            }        
        };
    }
}


#endif