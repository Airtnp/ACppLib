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

## `std::exchange`
```c++
template<class T, class U = T>
constexpr T exchange(T& obj, U&& new_value) noexcept(noexcept(...))
{
    T old_value = std::move(obj);
    obj = std::forward<U>(new_value);
    return old_value;
}

void SomeAsyncSubSystem::tick()
{
    for (const auto& callback : std::exchange(m_callbacks, {}))
    {
        callback();
    }
}

void SomeAsyncSubSystem::tick()
{
    PostToAnotherThread([v_ = std::exchange(m_callbacks, {})] () {
        for (const auto& callback : v_)
        {
            callback();
        }
    }
}

void SomeAsyncSubSystem::tick()
{
    auto v = (std::lock_guard<std::mutex>{m_mutex},
                std::exchange(m_callbacks, {}));
    for (const auto& callback : v)
    {
        callback();
    }
}

```

## Universal Memorization
```c++
template <typename T, T t, template <typename, typename> class TCacheContainer>
class Memoize;

template <typename TValue, typename ...TArgs, TValue (*f)(TArgs...), template <typename, typename> class TCacheContainer>
class Memoize<TValue(*)(TArgs...), f, TCacheContainer> {
    using TKey = std::tuple<typename std::decay<TArgs>::type...>;
    static TValue call(TArgs&&... args) {
        static TCacheContainer<TKey, TValue> cache;
        auto key = TKey{std::forward<TArgs>(args)...};
        auto res = cache.get(key);
        if (res != nullptr) {
            return *res;
        } else {
            auto value = f(std::forward<TArgs>(args)...);
            cache.set(key, value);
            return value;
        }
    }
};

```

### Lockfree Queue
```c++
bool push(int val) { 
    int prev = 0;
    geni ent;
    geni tmp;
    geni old = tmp = tail; // laxtomic load
    do {
        ent = buffer[tmp].load(relaxed);
        while( ! is_zero(ent, tmp.gen) ) {
            if (ent.gen < prev) {
                while(!tail.CAS(old,tmp) && old < tmp) { }
                return false; // full
            } else tmp.incr();
            if (ent.data) prev = ent.gen; }
            geni newg{val, tmp.gen};
    } while ( ! buffer[tmp].CAS(ent, newg, release));
    tmp.incr(); // go to next
    
    // update if no one else has gone as far:
    while (!tail.CAS(old, tmp) && old < tmp) { }
        return true;
}

int pop() {
    geni ent;
    geni tmp;
    geni old = tmp = head; // laxtomic load
    do {
        ent = buffer[tmp].load(relaxed);
        while( ! ent.is_data(tmp.gen) )
        if (ent.gen == tmp.gen) {
            while(!head.CAS(old,tmp) && old<tmp){ }
            return 0; // empty
        } else tmp.incr();
        geni zero{0, tmp.gen+1};
    } while ( ! buffer[tmp].CAS(ent, zero, acquire));
    tmp.incr(); // go to next
    
    // update if no one else has gone as far:
    while (!head.CAS(old, tmp) && old < tmp) { }
    return ent.val;
}

```

## Allocators, the good parts
![alloc](imgs/allocator-1.png)

```c++
class test_resource: public pmr::memory_resource
{
public:
    explicit test_resource(pmr::memory_resource*parent =
    pmr::get_default_resource());
    ~test_resource();
    pmr::memory_resource*parent() const;
    size_tbytes_allocated() const;
    size_tbytes_deallocated() const;
    size_tbytes_outstanding() const;
    size_tbytes_highwater() const;
    size_tblocks_outstanding() const;
    static size_tleaked_bytes();
    static size_tleaked_blocks();
    static void clear_leaked();
    // ...
protected:
    void *do_allocate(size_tbytes, size_talignment) override;
    void do_deallocate(void *p, size_tbytes,
    size_talignment) override;
    bool do_is_equal(constpmr::memory_resource& other)
                                const noexcept override;
private:
    // Record holding the results of an allocation
    struct allocation_rec{
        void *m_ptr;
        size_tm_bytes;
        size_tm_alignment;
    };
    pmr::memory_resource*m_parent;
    size_tm_bytes_allocated;
    size_tm_bytes_outstanding;
    size_tm_bytes_highwater;
    pmr::vector<allocation_rec> m_blocks;
    static size_ts_leaked_bytes;
    static size_ts_leaked_blocks;
};

void *test_resource::do_allocate(size_t bytes, size_t alignment) {
    void *ret = m_parent->allocate(bytes, alignment);
    m_blocks.push_back(allocation_rec{ret, bytes, alignment});
    m_bytes_allocated+= bytes;
    m_bytes_outstanding+= bytes;
    if (m_bytes_outstanding> m_bytes_highwater)
        m_bytes_highwater= m_bytes_outstanding;
    return ret;
}

void test_resource::do_deallocate(void *p, size_t bytes, size_t alignment) {
    // Check that deallocation argsexactly match allocation args.
    auto i= std::find_if(m_blocks.begin(), m_blocks.end(),
                        [p](allocation_rec& r){
                            return r.m_ptr== p; });
    if (i== m_blocks.end())
        throw std::invalid_argument("deallocate: Invalid pointer");
    else if (i->m_bytes!= bytes)
        throw std::invalid_argument("deallocate: size mismatch");
    else if (i->m_alignment!= alignment)
        throw std::invalid_argument("deallocate: Alignment mismatch");
    m_parent->deallocate(p, i->m_bytes, i->m_alignment);
    m_blocks.erase(i);
    m_bytes_outstanding-= bytes;
}

bool test_resource::do_is_equal(constpmr::memory_resource& other) const noexcept {
    // Two test resources are equal if they are the same resource.
    return this == &other;
}


template <typename Tp>
class slist{
public:
    using value_type= Tp;
    using reference = value_type&;
    using const_reference= value_typeconst&;
    using difference_type= std::ptrdiff_t;
    using size_type= std::size_t;
    using allocator_type= pmr::polymorphic_allocator<byte>;
    using iterator = …;
    using const_iterator= …;
    // Constructors
    slist(allocator_typea = {})
    : m_head(), m_tail_p(&m_head), m_size(0), m_allocator(a){}
    slist(constslist& other, allocator_typea = {});
    slist(slist&& other);
    slist(slist&& other, allocator_typea);
    
};

template <typename Tp>
template <typename... Args>
typename slist<Tp>::iterator slist<Tp>::emplace(iterator i, Args&&... args) {
    node* new_node= static_cast<node*>(
        m_allocator.resource()->allocate(sizeof(node), alignof(node)));
    try { 
        m_allocator.construct(std::addressof(new_node->m_value),
                                std::forward<Args>(args)...); 
    }
    catch (...) { 
        m_allocator.resource()->deallocate(new_node,
                                            sizeof(node), alignof(node));
        throw; 
    }
    new_node->m_next= i.m_prev->m_next;
    i.m_prev->m_next= new_node;
    if (i.m_prev== m_tail_p)
        m_tail_p= new_node; // Added at end
    ++m_size;
    return i;
}

template <typenameTp>
typename slist<Tp>::iterator slist<Tp>::erase(iterator b, iterator e) {
    node *erase_next= b.m_prev->m_next;
    node *erase_past= e.m_prev->m_next; // one past last erasure
    if (nullptr== erase_past)
        m_tail_p= b.m_prev; // Erasing at tail
    b.m_prev->m_next= erase_past; // splice out sublist
    while (erase_next!= erase_past) {
        node* old_node= erase_next;
        erase_next= erase_next->m_next;
        --m_size;
        m_allocator.destroy(std::addressof(old_node->m_value));
        m_allocator.resource()->deallocate(old_node,
        sizeof(node), alignof(node));
    }
    return b;
}

template <typenameTp>
slist<Tp>& slist<Tp>::operator=(const slist& other) {
    if (&other == this) return *this;
    erase(begin(), end());
    for (constTp& v : other)
        push_back(v);
    return *this;
}
template <typenameTp>
slist<Tp>& slist<Tp>::operator=(slist&& other) {
    if (m_allocator== other.m_allocator)
        swap(other); // non-copying move
    else
        operator=(other); // Copy assign
    return *this;
}

template <typenameTp>
slist<Tp>::slist(constslist& other, allocator_typea)
: slist(a) {
    operator=(other);
}

template <typenameTp>
slist<Tp>::slist(slist&& other)
: slist(other.get_allocator()) {
    operator=(std::move(other));
}

template <typenameTp>
slist<Tp>::slist(slist&& other, allocator_typea)
: slist(a) {
    operator=(std::move(other));
}
```