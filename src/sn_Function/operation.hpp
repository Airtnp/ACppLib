#ifndef SN_FUNCTION_OPERATION_H
#define SN_FUNCTION_OPERATION_H

namespace sn_Function {
    namespace operation {
        
        template <typename T, typename ...Args, template <typename...> typename C, typename F>
        auto map(const C<T, Args...>& container, const F& f) -> C<decltype(f(std::declval<T>()))> {
            using result_type = decltype(f(std::declval<T>()));
            C<result_type> res;
            for (const auto& item : container)
                res.push_back(f(item));
            return res;
        }

        template <typename T, typename ...Args, template <typename...> typename C, typename F, typename R>
        auto reduce(const C<T, Args...>& container, const R& start, const F& f) {
            R res = start;
            for (const auto& item : container)
                res = f(res, item);
            return res;
        }

        template <typename T, typename ...Args, template <typename...> typename C, typename F>
        auto filter(const C<T, Args...>& container, const F& f) {
            C<T, Args...> res;
            for (const auto& item : container)
                if (f(item))
                    res.push_back(item);
            return res;
        }

        template <typename F>
        auto compose(F&& f) {
            return [fx = std::forward<F>(f)](auto&&... args) {
                return fx(std::forward<decltype(args)>(args)...);
            };
        }

        template <typename F, typename ...Fs>
        auto compose(F&& f, Fs&&... fs) {
            return [fx = std::forward<F>(f), tail = compose(std::forward<Fs>(fs)...)](auto&&... args) {
                return fx(tail(
                    std::forward<decltype(args)>(args)...
                ));
            };
        }

        template <typename F>
        auto connect(F&& f) {
            return [fx = std::forward<F>(f)](auto&&... args) {
                return fx(std::forward<decltype(args)>(args)...);
            };
        }

        template <typename F, typename ...Fs>
        auto connect(F&& f, Fs&&... fs) {
            return [fx = std::forward<F>(f), tail = compose(std::forward<Fs>(fs)...)](auto&&... args) {
                return tail(fx(
                    std::forward<decltype(args)>(args)...
                ));
            };
        }
        
    }
}


#endif