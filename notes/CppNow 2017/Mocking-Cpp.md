# Mocking C++

[video](https://www.youtube.com/watch?v=t0wLm2iiEH0&index=12&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)

* vtable pointer -> RTTI + func() -> RTTI object vtp + name pointer -> 1X
* some void* -> some void* + 0 -> RTTI object vtp + name pointer -> runtime constructed string
* undefined in Cpp, unknown RTTI
* defined in ABI
* hippomock
* C++23 dreaming: reflection and reify
>> To "reify" something is to take something that is abstract and regard it as material. A classic example is the way that the ancients took abstract concepts (e.g. "victory") and turned them into deities (e.g. Nike, the Greek goddess of victory).

>> A reified type is a value that represents a type. Using reified types instead of real types means that you can do any manipulations with them that you can do with values.

>> In Haskell, the value undefined is a member of every (boxed) type, so that is often a good value to use to represent a type, assuming you don't need to break it apart.


```c++
struct T {
    virtual ~T() {}
};

struct rttiinfo {
    void* type;
    const char* name;
    void* base;
};

T* f(const char* name) {
    static rttiinfo rti;
    void** vt = new void*[10];
    static T object;
    memcpy(&rti, ((void***)&object)[0][-1], sizeof(rttiinfo));
    *(void**)&object = &vt[1];
    vt[0] = &rti;
    rti.name = name;
    return &object;
}

// mock object's rtti
int main(int, char** argv) {
    T* myObject = f(argv[1]);
    std::cout << typeid(*myObject).name() << "\n";
}
```

```c++
template <typename T>
class mock : public T {
    constexpr {
        for (auto f: $T.functions())
            if (f.is_virtual())
                $reify(f, [this](auto&& args...){
                    //...
                });
    }
};

template <typename T>
class proxy : public T {
    constexpr {
        for (auto f: $T.functions())
            static_assert(f.is_virtual());
            $reify(f, [this](auto&& args...){
                log(args...);
                inner.$name(f)(args...);
            });
    }
    logger(T& inner) {...}
};
```

* unit testing
* + macro to replace (invocable object)
* + mock functions
* + replace function at link or load time
* + - ODR violation(?)
* + - LD_PRELOAD_PATH (platform specific)
* + replace function itself at runtime
* + - memory protection error (cannot write to code section)
```c++
uint8_t* pMalloc = (uint8_t*)&malloc;
mprotect(
    (intptr_t)pMalloc & ~0xFFF,
    0x2000,
    PROT_READ | PROT_WRITE | | PROT_EXEC
);
pMalloc[0] = 0xE9;
```
* + - fit platform
* + - - x86 jump 5bytes
* + - - arm jump 12bytes
* + - - x86-64bit jump 14bytes
* + - may not be exact malloc
* + - - lib.so?
* + - - executable?
* + - may not be exact f()
* + - - inlined
* test all pending calls on function exit `VerifyAll()`