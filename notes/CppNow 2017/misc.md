# Misc

## String Terminology
[video](https://www.youtube.com/watch?v=wMyI-h9UY-M&index=10&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)

## Mathematical underpinnings of promises
[video](https://www.youtube.com/watch?v=2OY0Zn3oBCE&index=16&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)

* denotational semantics
* + u[promise<U>] = u[U]
* + u[promise<U>] = u[U] + Error
* + u[promise<U>] = T x u[U]
* + u[promise<U>] = T x (u[U] + Error)
* promise forms a Monad


## Radix sort

[video](https://www.youtube.com/watch?v=zqs87a_7zxw&index=24&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)

### SKA Sort
* shifts/bit masks/pointer arithmetic
* counting sort for bytes
* prefix sum
* swapping
* American flag sort
* Dutch flag problem

## Application of reflection
```c++
template <typename T>
struct concept_map {
    constexpr {
        for...(const auto& function: $I.member_functions()) {
            using F = decltype(function);
            F::return_type iddecl(function)(F::arguments&& args) {
                return poly_.virtual_(stringize(function.name()))(poly, args);
            }
        }
    };
private:
    poly<Concept> poly_;
};
``` 

## Customization point

* typeclass
* virtual template
* concept maps
* type erase
* reflection

## TODO

### Multicore Synchronization
[video](https://www.youtube.com/watch?v=OfTy3ymDwWE&index=39&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)

### Std2
[video](https://www.youtube.com/watch?v=fjtwfauk7a8&index=27&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)