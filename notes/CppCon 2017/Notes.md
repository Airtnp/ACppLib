# Notes - Miscs

## End-to-end Deadlock Debugging

### folly::Synchronized (develop)
```c++
class Counter {
public:
    void add(int n) {
        unique_lock<shared_mutex> g(mutex_);
        counter_ += n;
        deltas_.push_back(n);
    }
    int get() const {
        shared_lock<shared_mutex> g(mutex_);
        return counter_;
    }
private:
    mutable shared_mutex mutex_;
    int counter_;
    vector<int> deltas_;
};

```

```c++
class Counter {
public:
    void add(int n) {
        state_->wlock([&n](auto& state) {
            state.counter += n;
            state.deltas.push_back(n);
        });
    }
    int get() const {
        return state_->rlock([](const auto& state) {
            return state.counter;
        });
    }
private:
    struct State {
        int counter;
        vector<int> deltas;
    };
    folly::Synchronized<State, shared_mutex> state_;
};
```


### ThreadSanitizer Testing (production and develop)
* TSAN
* Cycle in lock order graph

### eBPF Deadlock Detector (monitor)
* Linux eBPF (Linux Extended Berkeley Packet Filter)
* kernel-virtual
* hooks
* export shared data structure from kernel to userspace
* construct wait-for graph (find non-DAG)

### gdb Deadlock Extension (post-moden)
1. Find all threads stuck on
pthread_mutex_lock
2. For each blocked thread A
    a) Find the owner thread B
    of the mutex
    b) Add directed edge (A, B)
3. Deadlock exists iff cycle exists

![compare](imgs/end-to-end-1.png)

## MPark.MPatterns
```c++
template <typename... Patterns>
struct Pattern {
    template <typename F>
    Case<Pattern, F> operator=(F &&f) && noexcept;
    
    Ds<Patterns &&...> patterns;
};

typename Pattern, typename F>
struct Case {
    Pattern pattern;
    F f;
};

template <typename T>
struct match_result : optional<forwarder<T>> {
    using type = T;
    using super = optional<forwarder<T>>;
    using super::super;
    match_result(no_match_t) noexcept {}
    match_result(nullopt_t) = delete;
    decltype(auto) get() && {
        return (*static_cast<super &&>(*this)).forward();
    }
};

template <typename F, typename... Args>
auto match_invoke(F &&f, Args &&... args) {
    using R = invoke_result_t<F, Args...>;
    if constexpr (is_void_v<R>) {
        invoke(forward<F>(f), forward<Args>(args)...);
        return match_result<void>(void_{});
    } else if constexpr (is_match_result_v<R>) {
        return invoke(forward<F>(f), forward<Args>(args)...);
    } else {
        return match_result<R>(
        invoke(forward<F>(f), forward<Args>(args)...));
    }
}

template <typename ExprPattern, typename Value, typename F>
auto try_match(const ExprPattern &expr_pattern,
                Value &&value,
                F &&f) {
    return expr_pattern == forward<Value>(value)
        ? match_invoke(forward<F>(f))
        : no_match;
}

template <typename Pattern, typename Value, typename F>
auto try_match(const Arg<Pattern> &arg,
                Value &&value,
                F &&f) {
    if constexpr (is_void_v<Pattern>) {
        return match_invoke(forward<F>(f),
                            forward<Value>(value));
    } else {
        return try_match(arg.pattern,
                        forward<Value>(value),
                        forward<F>(f));
    }
}

template <typename... Values>
struct Match {
    template <typename Pattern, typename F, typename... Cases>
    decltype(auto) operator()(Case<Pattern, F> &&case_,
    Cases&&... cases) && {
        auto result = try_match(move(case_).pattern.patterns,
                                move(values),
                                move(case_).f);
        if (result) {
            return move(result).get();
        }
        if constexpr (sizeof...(Cases) == 0) {
            throw match_error{};
        } else {
            return move(*this)(forward<Cases>(cases)...);
        }
    }
    tuple<Values &&...> values;
};


```