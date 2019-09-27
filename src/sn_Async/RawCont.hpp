/// Taking from CppCon2019, A unifying abstraction for async

#ifndef SN_ASYNC_RAWCONT_H
#define SN_ASYNC_RAWCONT_H

#include "../sn_CommonHeader.h"
#include "../sn_Concept.hpp"

namespace sn_Async {
    using namespace sn_Concept;
    namespace raw_cont {
        template <typename P, typename F>
        struct then_promise {
            P p_;
            F fun_;
            void set_value(auto ...vs) {
                p_.set_value(fun_(vs...));
            }
            void set_exception(auto e) {
                p_.set_exception(e);
            }
        };
        template <class P, class F>
        then_promise(P, F) -> then_promise<P, F>;

        auto then(auto task, auto fun) {
            return [=](auto p) {
                task(then_promise{p, fun});
            };
        }

        struct sink {
            void set_value(auto...) {}
            void set_exception(auto) {
                std::terminate();
            }
        };

        template <typename T>
        struct sync_state {
            std::mutex mtx;
            std::condition_variable cv;
            std::variant<std::monostate, std::exception_ptr, T> data;
        };

        template <typename T>
        struct sync_promise {
            sync_state<T>* pst;
            template <size_t I>
            void set(auto... xs) {
                auto lk = std::unique_lock(pst->mtx);
                pst->data.template emplace<I>(xs...);
                pst->cv.notify_one();
            }
            void set_value(auto... xs) {
                set<2>(xs...);
            }
            void set_exception(auto e) {
                set<1>(e);
            }
        };


        template <typename T, class Task>
        T sync_wait(Task task) {
            sync_state<T> state;
            task(sync_promise<T>{&state});
            {
                auto lk = std::unique_lock{state.mtx};
                state.cv.wait(lk, [&state] {
                    return state.data.index() != 0;
                });
            }
            if (state.data.index() == 1)
                std::rethrow_exception(std::get<1>(state.data));
            return std::move(std::get<2>(state.data));
        }

        template <class P, class E = std::exception_ptr>
        concept Receiver = requires(P& p, E&& e) {
            p.set_error(std::forward<E>(e));
            p.set_done();
        };

        template <class P, class ...Vs>
        concept ReceiverOf = Receiver<P> && Invocable<P, Vs...>;



        namespace test {
            auto thread_executor() {
                return [](auto p) {
                    std::thread t {
                            [p = std::move(p)] () mutable {
                                p.set_value();
                            }
                    };
                    t.detach();
                };
            }


            auto async_algo(auto task) {
                return then(task, [] {
                    int answer = 1;
                    return answer;
                });
            }

            void test() {
                auto f = async_algo(thread_executor());
                auto f2 = then(f, [](int i) {
                    return i + rand();
                });
                auto res = sync_wait<int>(f2);
            }
        }
    }
}







#endif