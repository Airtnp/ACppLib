# Notes - Miscs

## End-to-end Deadlock Debugging

### folly::Synchronized (develop)
```c++
void doubleValues(Synchronized<vector<int>>& vec) {
    auto locked = vec.wlock();
    for (int& n : *locked) { // iterator lock the loop
        n *= 2;
    }
}
```

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

## Build C++ Modules
```c++
export module hello.core;
export
{
    enum class volume {quiet, normal, loud};
    void say_hello (const char*, volume);
}

export namespace hello
{
    enum class volume {quiet, normal, loud};
    void say (const char*, volume);
}
namespace hello
{
    void impl (const char*, volume); // Not exported.
}

// hello.extra interface
//
export module hello.extra;
import hello.core; // Exports say_hello().
// hello.extra implementation
//
module hello.extra;
say_hello (”World”); // Ok.

// re-export
export module hello;
export
{
    import hello.core;
    import hello.basic;
    import hello.extra;
}

// hello.mxx
export module hello;
export void say_hello (const char* name);
// hello.cxx
#include <iostream>
module hello;
void say_hello (const char* n)
{
    std::cout << ”Hello, ” << n << ’!’ << std::endl;
}
// driver.cxx
import hello;
int main () { say_hello (”Modules”); }
```

* interface template
```
<header includes>
export module <name>; // Start of module purview.
<module imports>
<special header includes> // Config, export, etc.
export
{
    <module interface>
}
<inline/template includes>

#ifndef __cpp_modules
#pragma once
#endif
// C includes, if any.

#ifndef __cpp_lib_modules
    <std includes>
#endif

// Other includes, if any.
#ifdef __cpp_modules
export module <name>;
#ifdef __cpp_lib_modules
    <std imports>
#endif
#endif
```
* implementation template
```
<header includes>
module <name>; // Start of module purview.
<extra module imports> // Only additional to interface.
<module implementation>

#ifndef __cpp_modules
#include <module interface file>
#endif

// C includes, if any.
#ifndef __cpp_lib_modules
    <std includes>
    <extra std includes>
#endif

// Other includes, if any
#ifdef __cpp_modules
    module <name>;
#ifdef __cpp_lib_modules
    <extra std imports> // Only additional to interface.
#endif
#endif
```
* consumer
```
#ifdef __cpp_modules
import hello;
#else
#include <libhello/hello.mxx>
#endif
```

## Persistent Cpp
```c++
struct entry {
    persistent_ptr<entry> next;
    p<int> value;
};

void push(pool_base& pool, int value) {
    transaction::exec_tx(pool, [this]{
        auto n = make_persistent<entry>(value, nullptr); // persistent_ptr
        if (head == nullptr) {
            head = tail = n;
        } else {
            tail->next = n;
            tail = n;
        }
    });
}

int pop(pool_base& pool) {
    transaction::exec_tx(pool, [this]{
        if (head == nullptr) {
            throw runtime_error("Nothing to pop");
        } 
        auto res = head->value;
        auto tmp_entry = head->next;
        delete_persistent<entry>(head);
        head = tmp_entry;
        if (head == nullptr)
            tail = nullptr;
        return ret;
    });
}
```

## Coroutine
```c++

int result = hard_work();
co_return result;
// ----->
using promise_type =
    coroutine_traits<lazy<int>>::promise_type;
{
    promise_type p;
    auto r = p.get_return_object();
    co_await p.initial_suspend();
    try {
        int result = hard_work();
        p.return_value(result);
        goto final_suspend;
    } catch (...) {
        p.unhandled_exception();
    }
final_suspend:
    co_await p.final_suspend();
destroy:
}

auto y = co_await x;
// ----->
auto h = coroutine_handle<P>::from_promise(p);
auto a = p.await_transform(x); // optional
auto e = operator co_await(a); // optional
if (!e.await_ready()) {
    // suspended
    e.await_suspend(h);     
    if (/* still suspended */) {
        // return to caller
resume:
    }
// resumed
}
auto y = e.await_resume();

auto y = co_yield x;
// ----->
auto h = coroutine_handle<P>::from_promise(p);
auto a = p.yield_value(x);
auto e = operator co_await(a); // optional
if (!e.await_ready()) {
    // suspended
    e.await_suspend(h);
    if (/* still suspended */) {
    // return to caller
resume:
    }
// resumed
}
auto y = e.await_resume();

for co_await (auto x : xs) {
    use(x);
}
// ----->
{
    auto __end = xs.end();
    for (auto __begin = co_await xs.begin();
        __begin != __end;
        co_await ++__begin) {
        auto x = *__begin;
        use(x);
    }
}

template <typename Promise>
struct coroutine_handle {
    constexpr explicit
    operator bool() const noexcept;
    Promise& promise() const;

    static coroutine_handle
        from_promise(Promise&);

    void resume();
    void destroy();
};

template <typename T>
struct lazy_promise;

template <typename T>
class lazy {
private:
    using handle_type =
    stdx::coroutine_handle<lazy_promise<T>>;
    lazy(lazy_promise<T>& p)
        : h(handle_type::from_promise(p)) {}
    
    handle_type h;
    std::optional<T> value;
    
    friend lazy_promise<T>;
public:
    T get() {
        if (!value) {
            h.promise().dest = &value;
            h.resume();
            h = {};
        }
        return *value;
    }
    ~lazy() {
        if (h) h.destroy();
    }
};

template <typename T>
struct lazy_promise {
    std::optional<T>* dest;
    
    auto get_return_object() {
        return lazy<T>(*this); 
    }
    
    auto initial_suspend() {
        return stdx::suspend_always{}; 
    }
    
    void return_value(T x) {
        *dest = std::move(x); 
    }
    
    void unhandled_exception() {}
    
    auto final_suspend() {
        return stdx::suspend_never{}; 
    }
};

template <typename T, typename... Args>
struct coroutine_traits<lazy<T>, Args...> {
    using promise_type = lazy_promise<T>;
};
```
* prime generator
```c++
task<void> async_print_primes() {
    for co_await (auto x : async_primes()) {
        print(x);
        if (x > 100) break;
    }
}

task<bool> is_prime_async(int);

async_generator<int> async_primes() {
    for (int x = 0; ; ++x)
        if (co_await is_prime_async(x))
            co_yield x;
}
```
* `optional<T>`
```c++
template <typename T, typename... Args>
struct coroutine_traits<optional<T>, Args...> { // UB!
    using promise_type = optional_promise<T>;
};

template <typename T>
struct optional_promise {
    template <typename U>
    auto await_transform(optional<U> e) {
        return optional_awaitable<U>{move(e)};
    }
};

template <typename U>
struct optional_awaitable {
    optional<U> o;
    auto await_ready() { return o.has_value(); }
    auto await_resume() { return o.value(); }
    template <typename T>
    void await_suspend(coroutine_handle<maybe_promise<T>> h) {
        h.promise().data->emplace(nullopt);
        h.destroy();
    }
};
```
* callable
```c++
template <typename Ret,
typename... Args>
class func {
    template <typename F>
    static func create(F f) {
        co_yield f;
    }
    Ret operator()(Args… args) {
        h.promise().args = {args…};
        h.resume();
        return h.promise().ret;
    }
};

template <typename Ret,
typename... Args>
struct func_promise() {
    tuple<Args…> args;
    Ret ret;
    template <typename F>
    void yield_value(F f) {
        ret = apply(f, args);
    }
};
```

## Implement `dynamic_cast`
```c++
void *truly_dynamic_to_mdo(void *p) {
    uint64_t *vptr =
        *reinterpret_cast<uint64_t **>(p);
    uint64_t mdoffset = vptr[-2];
    void *adjusted_this =
        static_cast<char *>(p) + mdoffset;
    return adjusted_this;
}

type_info& dynamic_typeid(void *p)
{
    return *(*reinterpret_cast<type_info ***>(p))[-1];
}

template<class To, class From>
To *truly_dynamic_from_base_to_derived(From *p) {
    void *mdo = truly_dynamic_to_mdo(p);
    const type_info& ti = dynamic_typeid(mdo);
    int offset = (char *)p - (char *)mdo;
    if (ti == From) {
        return nullptr;
    } else if (ti == To && ti.isPublicBaseOfMe(offset, From)) {
        return (To *)(mdo);
    } else if (void *so = ti.maybeFromHasAPublicChildOfTypeTo(
                                mdo, offset, From, To)) {
        return (To *)(so);
    } else if (ti.isPublicBaseOfMe(offset, From)) {
        return (To *)(ti.castToBase(mdo, To));
    }
    return nullptr;
}

template<class To, class From>
To *truly_dynamic_between_unrelated_classes(From *p) {
    if constexpr (is_final_v<To> || is_final_v<From>) {
        return nullptr;
    }
    void *mdo = truly_dynamic_to_mdo(p);
    const type_info& ti = dynamic_typeid(mdo);
    int offset = (char *)p - (char *)mdo;
    if (ti == From) {
        return nullptr;
    } else if (ti == To) {
        return (To *)(mdo);
    } else if (ti.isPublicBaseOfMe(offset, From)) {
        return (To *)(ti.castToBase(mdo, To));
    }
    return nullptr;
}

template<class P, class From, class To = remove_pointer_t<P>>
To *dynamicast(From *p) {
    if constexpr (is_same_v<From, To>) {
        return p;
    } else if constexpr (is_base_of_v<To, From>) {
        return (To *)(p);
    } else if constexpr (is_void_v<To>) {
        return truly_dynamic_to_mdo(p);
    } else if constexpr (is_base_of_v<From, To>) {
        return truly_dynamic_from_base_to_derived<To>(p);
    } else {
        return truly_dynamic_between_unrelated_classes<To>(p);
    }
}
```

## Function default arguments (DFA)
> Expressions that are evaluated when there are fewer provided arguments to a function call than the number of parameters specified in the function definition.

> Default function arguments can not...
● appear in operator functions (except operator())
● appear in a position where there is already a visible DFA
● appear in a position where all parameters to the right do not have effective DFAs
● appear in out-of-class definitions of member functions of class templates
● appear in a friend declaration, unless that declaration is the only one in the translation unit and is an in-class friend function definition
● appear in declarations of
○ pointers or references to functions
○ type alias declarations
● appear in requires expressions (concepts)
● appear in explicitly defaulted member functions
● appear in user-defined literal declarations/definitions
● be provided for the first parameter of special member functions
● be provided for the first parameter of an initializer_list constructor
● be provided for the size_t parameter of allocation functions (i.e. new)
● be provided for a parameter pack
● be used to deduce a template type-parameter
● differ for an inline function defined in multiple translation units

> Default argument expressions can not contain...
● a lambda that captures (by-value or by-reference and implicit or explicit does not matter)
● a function-local variable unless in an unevaluated context
● the this keyword
● non-static class members (with few exceptions)
● previously declared parameter names unless in an unevaluated context (but they are in scope!)

* replacement: overloading
* + ambiguous lookup

> Names in a DFA are bound at declaration, but evaluated at use
```c++
namespace N {
    string fn(string s = b) { return s; } // valid!
    string b = "bar";
}
```

> DFAs can be provided across multiple declarations of the same function
The first declaration in a scope hides any previously provided DFAs
```c++
auto fn(string s, bool b = true) {
    return b ? s : "";
}
auto fn (string s = "foo", bool b); // Now fn() is valid! non-overlap
{
    auto fn(string, bool); // Now fn() and fn("foo") no longer valid
}
```

> Restrictions on DFAs across multiple declarations
For each parameter for function F, there may be only a single declaration that provides a DFA.
● A parameter P that has a DFA in a declaration for function F, is allowed only if there are visible DFAs for all following parameters for function F.
● Within scope S, the first declaration for function F hides any previously visible DFAs for function F within scope S. 
● For function F, called within scope S, the effective DFAs for F are the union of all visible DFAs at the call-site.

> DFAs on base member functions are visible unless you hide them with your own member function declarations but you can unhide them with `using` declarations in the class definition
```c++
struct Base { auto fn(string s = "foo") { return s; }};
struct D_1 : Base {
    void fn(char);
    using Base::fn;
};

struct D_2 : Base {
    using Base::fn;
    auto fn(string s = "bar") { return s + "!!!" }
};
```

* virtual member function do not inherit whether it has default argument, but if you declare both of them, it will call on the static type

> function default argument template
For a function template (and presumably a member function of a class template), DFAs are not always completely parsed until the template has been called in a way that requires the DFA.

```c++
void my_assert(
    bool test, const char* reason, 
    std::source_location loc = std::source_location::current()) {
    if (!test) {
        std::cout << "Assertion failed: " << loc.file_name << ":"
                  << loc.line << ":" << loc.column << ": "
                  << "in function " << loc.function_name >> ": "
                  << reason << '\n';
    }
}

enum class tag { complietime, runtime };
tag runtime() { return tag::runtime }
int identity(int n) { return n; }

constexpr int fn(int n, tag t = runtime()) {
    return (t == tag::runtime) ? identity(n) : n * n;
}
```

## From Secure to GPU
* secure_allocator
* + allocation: locks memory (cannot be swapped out) (mlock+madvise/VirtualLock)
* + deallocation: wipes memory before freeing (OPENSSL_cleanse, RtlSecureZeroMemory, etc)

## ThinLTO
* Monolithic LTO
* + Link all bitcode to one single module
* + Removes module optimization boundaries via Cross-Module Optimization (CMO)
* + Most of benifit comes from cross-module inlining
* + Binary Size: inherent dead=stripping and auto hidden visibility via internalization
* + Single source improvements because global variables can be internalized (better alias analysis, etc.).
* + slow / not friendly with incremental build / memory hungry
* ThinLTO
* + parallel / incremental / memory lean
* + Phase1 Compile: fully-parallel frontend-processing + initial optimization
* + - extra per-function summary information
* + - - includes local reference/call graph
* + - - includes module hash (for incremental)
* + Phase2 Thin Link: Link only the summary info in a giant index: thin-link
* + - includes full reference/call graph to enable inter-procedural analysis (IPA)
* + - no need to parse the IR
* + - serial phase (but fast)
* + Phase3 ThinLTO Backends: parallel inter-procedural transformation based on the analysis results
* + - includes cross-module function importing (dropped after inlining)
* + - fully-parallel
* + - Distributed can be inserted on Phase3
* + - hash per job input and use .o cache system for output of Phase2 to do incremental
* + Integration with Bazel
* + Optimization
* + - analysis during Thin Link
* + - - full call/reference graph from summaries
* + - - results recorded in the index (e.g. linkage type changes)
* + - transformation during parallel backends
* + - - applies results of whole program analysis performed during Thin Link
* + - - independently applied in each backend
* + WPA (weak linkage resolution)
* + - dead global pruning
* + - Linker identifies external reference to getGlobalOption()
* + - Compute reachability to externally referenced nodes in index
* + - Prune unreachable nodes from the graph
* + - Enabler for better subsequent analysis
* + - + Option can be internalized and later constant folded.
* + - + The function-importing will generate a smaller list save CPU cycles!
* + PGO (profile guided optimization)
* + - More likely to inline small cold function without profile data
* + - With profile data, call hotness is known
* + - - Annotate edges in thin link graph with hotness
* + - - Import hot calls more aggressively to match inlining heuristics
* + - Inline hot calls more aggressively for better optimization of hot pat
* + - More likely to import small cold function without profile data
* + - - Reduce compile time by avoiding needless importing
* + - Indirect Call Promotion
* + - - The summary records hot indirect call targets as regular calls (speculative)
* + - - Hot indirect call targets imported, available for promotion & inlining
* GCC WHOPR
* + WPA (serial) makes IPA and inlining decision, rewrites partitioned IR
* + - Comparable Phase 2: Thin Link
* + LTRANS (parallel) Parallel backends performing inlining within each partition, plus usual optimizations and code generation
* + - Comparable Phase 3: ThinkLTO
* Advise
* + Common advice: Put definitions of small functions in headers (using inline keyword to avoid ODR errors)
* + - Enables inlining into callers
* + - Compile and possibly codegen in every #include-ing translation unit
* + With ThinLTO: Function definitions can stay in implementation file (caveats)
* + - Compile one copy (to IR), and codegen one copy (to object)
* + - Imported and optimized only where needed
* + Caveats (functions with vague linkage that need to be defined in header):
* + - Template functions (unless explicitly specialized)
* + - Functions with the inline keyword - just remove it
* LTCG

## When 1ms counts
* low latency
* slowpath removal
* template=based (compile-time) configuration
* prefer lambda
* reuse object instead of deallocating
* delete from another thread
* no exception for control flow
* pre templates to branches
* multi-thread (not do it)
* + synchronization via locking is expensive
* + lock free requires lock at hardware level
* + complex to implement correctly
* + easy for accident
* + must use
* + - keep shared data to absolute minimum
* + - consider passing copies rather than sharing
* + - share -> consider not use synchronization
* hash
* + hybrid of chainning and open addressing
* + - predictable cache access patterns
* + - prefetched candidate hash values
* ((always_inline))
* keep cache hot
* + don't share L3 (disable all but 1 core) (or lock the cache)
* + multi cores -> check neighbours (noisy neighbours should moved to difference physical CPU)
* placement new slightly inefficient (gcc < 7.1 without -std=c++17)
* + gcc/clang low version do null-pointer check on memory
* SSO
* + avoid allocation with (gcc > 5.1 or < 15 chars | clang if < 22 chars>)
* + However ABI compatible linux distribution will using old COW string
* static local variable initialization
* + atomic init (even binary single-threaded)
* [inplace-function](https://github.com/WG21-SG14/SG14/blob/master/SG14/inplace_function.h) over std::function (may copy and allocate)
* std::pow may be slow (transcendental)

## DLLS
* dynamic link library
* + contains code data
* + can be loaded dynamically at runtime (defer load functionality)
* + can be shared or reused by multiple programs (reduce disk/memory usage)
* + benefits
* + - componentization via DLLs
* + - improved service ability (hotfix, patching)
* + - improve maintain ability
* + disadvantages
* + - complicated software distribution
* + - increase potential for incompatibilities
* + - impossible to optimize code across DLL boundaries (indirect)
* Build a DLL
* + `cl /c && link /DLL /NOENTRY /EXPORT:foo`
```c++
#include <Windows.h>
#include <stdio.h>
int main() {
    HMODULE const HelloDLL = LoadLibraryExW(L"Hello.dll", nullptr, 0);
    // char const* __cdecl foo()
    using foo_t = char const* (__cdecl*)();
    foo_t const foo = reinterpret_cast<foo_t>(
        GetProcAddress(HelloDLL, "foo")
    );
    puts(foo());
    FreeLibrary(HelloDLL);
}
```
* Inside a DLL
* + DOS Stub: Valid DOS program
* + - offset 3C = offset of PE Signature
* + PE Signtature: "PE\0\0"
* + COFF File Header: Common file header
* + - machine type
* + - number of sections
* + - time date stamp
* + - size of optional header
* + - characteristics (Executable / Application can handle large address / DLL)
* + "Optional" Header: image-specific file headers
* + - magic (PE32+)
* + - entry point
* + - image base
* + - section alignment
* + - file alignment
* + - size of image
* + - size of headers
* + - DLL characteristics (High Entropy Virtual Address / Dynamic base / NX compatible)
* + - number of directories
* + - RVAs
* + - - RVA of export directory
* + - - RVA of import directory
* + - - RVA of resource directory
* + - - RVA of exception directory
* + - - RVA of debug directory
* + - - more empty directories
* + Section Headers: contain information about "sections" in the DLL
* + Sections 0-N: contains the actual code/data/resources in the DLL
* + - .text
* + - - virtual size
* + - - virtual address
* + - - size of raw data
* + - - file pointer to raw data
* + - - flags (Code / Execute Read / Initialized Data / Read Only)
* + - .rdata
* RVA: relative virtual address
* + offset from the beginning of the DLL
* + address in memory = DLL Base Address + RVA
* Implicit Linking and Import
* + explicit: LoadLibraryExW/GetProcAddress
* + implicit: by compile option (.lib)
* Specifying Exports
* + `/EXPORT:Out, PRIVATE=Rename`
* + `.def` + `/DEF:x.def`
* + `__declspec(dllexport)`
* + `#pragma comment(linker, "/export:foo")`
* Load process
* + - find dll
* + - map dll to memory
* + - load dll that depends
* + - bind import from dependents
* + - call entry point to let dll initialized itself
* + DLLs are Reference Counted
* Path to load
* + same name: return first one
* + known: system path
* + search order
* + - directory from which the application loaded
* + - system `\System32` | `\SysWOW64`
* + - 16bit system directory `\System`
* + - Windows directory `\Windows`
* + - current directory
* + - PATH listed in `%PATH%`
* + Customize
* + - DLL Redirection (`.local`)
* + - Side-by-Side Components
* + - `%PATH%`
* + - `AddDllDirectory`
* + - `LoadLibraryEx` Flags
* + - - LOAD_WITH_ALTERED_SEARCH_PATH/LOAD_LIBRARY_SEARCH_XXX
* + - WIndows Store / UWAs are different
* Mapping the DLL into memory
* + different alignment on disk (FAT sector) and memory (page size)
* + sections must be page-aligned in memory
* + Loader
* + - open DLL file and read image size
* + - allocate a continguous, page-aligned block of memory of that size
* + - copy the contents of each section into the appropriate aread of that block of memory
* Relocation
* + Pointer = Pointer - PreferredBaseAddress + ActualBaseAddress
* Load Dependencies and Binding Imports
* + for each DLL dependency
* + - load the DLL...
* Initializing the DLL
* + `BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)`
* + - `instance` handle to the DLL, same as return from `LoadLibrary`
* + - `reason`why calling entry point
* + - - `DLL_PROCESS_ATTACH`
* + - - `DLL_PROCESS_DETACH`
* + - - `DLL_THREAD_ATTACH`
* + - - `DLL_THREAD_DETACH`
* + - `reserved`
* + - - process-attach: null if DLL loaded via `LoadLibrary`; non-null if implicit
* + - - process-detach: null if DLL loaded via `FreeLibrary`; non-null if process terminating
* + - call to `DllMain` are synchronized by a global lock, called the Loader Lock
* + Best practice
* + - do as little as possible in your entry point
* + - be careful when calling into other DLLs from your entry point
* + - do not synchronize with other threads from your entry point
* Diagnosing DLL Load Failures
* + `gflags /i x.exe +sls`
* `__declspec(dllimport)` -> can be inlined without direction
* Exporting Data
* + `/export:var,DATA`
* + `__declspec(dllimport)` is forced for data imports
* + or your can `GetProcAddress`
* 共享节
* Delay Loading
* + `/DELAYLOAD:x.dll delayimp.lib`
* + load by need
* + generate `__imp_load_foo()`
* + call `__tailMerge_DllWithEntryPoint_dll(&__impl_foo)`
* + - push parameter registers onto the stack
* + - call `__delayLoadHelper2`
* + - - `HANDLE DllWithEntryPointHandle = LoadLibrary(DllWithEntryPoint.dll)`
* + - - `__imp_foo = GetProcAddress(DllWithEntryPointHandle, "foo")`
* + - pop parameter registers back off of the stack
* + - jump to `__imp_foo` (transfer control to the now-imported function)
* + - raise SEH if failed
* C++ and DLL
* + global variable -> use `__DllMainCRTStartup` to construct
* + `__DllMainCRTStartup`
* + - C Runtime(CRT) provides an entry point
* + - handles initialization of C/C++ language feature
* + - At `DLL_PROCESS_ATTACH`
* + - - If CRT is statically linked into the DLL, initializes the statically linked CRT
* + - - Initializes security cookies and other run-time check support
* + - - Runs constructors for global variables
* + - - Initializes `atexit()` support within the DLL
* + - - calls user-defined `DllMain` if one is defined
* + - At `DLL_PROCESS_DETACH`
* + - - call `atexit()`-registed functions
* + - - runs destructors for global variables
* + - - If CRT is statically linked into the DLL, shuts down the statically linked CRT
* + Exporting C++ functions/classes ?
* + - don't (mangling/ABI)
* + - extern "C"
* Threads and TLS
* + remeber `DLL_THREAD_ATTACH/DETACH`
* + TLS var that zero-init works fine
* + TLS var that can be statically-init work fine
* + TLS var that requires dynamic initialization
* + - `thread_local unsigned int y = GetCurrentThreadId()`
* + - only correctly initialized for
* + - - thread that loaded the DLL
* + - - any threads after the DLL is loaded
* + - for threads before DLL is loaded -> zero-init
* Avoiding DLL Hell
* + UWP/ Centeniial App Package
* + Maintain API/ABI Stability
* + Breaking change -> rename the DLL