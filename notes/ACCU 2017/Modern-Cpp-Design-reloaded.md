# Modern Cpp Design reloaded

[video](https://www.youtube.com/watch?v=jkjXIh3E9v0)

## TypeList
* new list
```c++
template <typename...>
struct list {};
```
* push_back
```c++
template <typename Input, typename C = listify>
struct push_back {
    template <typename ...Ts>
    using f = typename C::template f<Ts..., Input>;
}
```
* example
```c++
// composable
template <typename ...Ts>
using foo = push_back<int, push_back<float>>::template f<Ts...>;
using result = foo<float>;
uisng result2 = foo<char>;
```

## Smart pointers/Small object allocation/Abstract Factory

## Concept-based-polymorphism
* Concept
```c++
class drawable_concept {
    public:
        drawable_concept() = default;
        virtual ~drawable_concept() = default;
        virtual void draw() = 0;
}

template <typename T>
class drawable_model : public drawable_concept {
    T model_;
    public:
    drawable_model(const T& model) : model_(model) {}
    void draw() {
        model_.draw();
    }
}
```
* Concept 2
```c++
class drawable {
    std::unique_ptr<drawable_concept> object_;
    public:
    template <typename T>
    drawable(const T& x) : object_(new drawable_model<T>(x)) {}
    void draw() {
        object_->draw();
    }
}

struct my_widget {
    void draw() {
        //...
    }
}

int main() {
    drawable d(my_widget{});
}
```
* [dyno](https://github.com/ldionne/dyno)
* + Type-erasing Polymorphism

## Policy Based Class Design

### PB
```c++
template <typename OutputPolicy, typename InputPolicy>
class Foo : private OutputPolicy, private InputPolicy {
    using OutputPolicy::out;
    using InputPolicy::in;
    void bar(Ty s) {
        out(in(s));
    }
};
```
* Variations (Problems)
* + policy class add to the public interface
* + access the data and the interface of other policies internally
* + change behavior depending on other policies
* + initialize policies with state

### CRTP
```c++
template <typename OutputPolicy, typename InputPolicy>
class Foo : public OutputPolicy<Foo, InputPolicy>, InputPolicy<Foo, OutputPolicy> {
    void bar(Ty s) {
        out(in(s));
    }
    // out -> outputpolicy
    // in -> inputpolicy
};
```
* Drawbacks
* + bad for vocabulary types
* + verbose typename for complex types
* + poluted public interface
* + complicated to construct when stateful
* + bad encapsulation

### Genetic combiner
```c++
// oh, c++1z inheritance
// take policies as pack
template <typename ...Ts>
class combiner : public Ts::template f<combiner<Ts...>>... {
    std::tuple<Ts...> data;
    public:
        combiner(std::tuple<Ts...> d) : data(std::move(d)) {}
        friend class policy_access<combiner>;
};

template <typename T>
struct policy_access {
    decltype(T::data)& data;
    template <typename U>
    policy_access(U* p)
        : data(static_cast<T*>(p)->data){}
    template <typename U>
    decltype(std::get<U>(data))& operator[] (index_t<U>) {
        return std::get<U>(data);
    }
};

struct CRTP_C : public policy_access<combiner<A, B, C>> {}

// CRTP_C a; a.foo();

struct T_policy;

template <typename T>
struct T_public_policy {
    void foo() {
        policy_access<T>(this)[idx<T_policy>].i_++;
    }
};

struct T_policy {
    T i_;
    template <typename T>
    using f = T_public_policy<T>;
};

```