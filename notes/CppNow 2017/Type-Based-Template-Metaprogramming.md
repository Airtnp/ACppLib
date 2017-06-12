# Typed Based Template Metaprogramming is Not Dead

[video](https://www.youtube.com/watch?v=EtU4RDCCsiU&index=20&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)

* brigand
```c++
template <typename L>
using cart_prod = reverse_fold<
                        L, 
                        list<list<>>,
                        l::join<
                            l::transform<
                                _2,
                                pin<
                                    1::join<
                                        1::transform<
                                            super<_1>,
                                            pin<
                                                list<
                                                    1::push_front<
                                                        _1, 
                                                        super<_1>
                                                    >
                                                >
                                            >
                                        >
                                    >
                                >
                            >
                        >
                    >;

```

* cost of operations
* + SFINAE
* + instantiating a function template
* + instantiating a type
* + calling an alias
* + adding a parameter to a type
* + adding a parameter to an alias call
* + looking up a memoized type
* fold/replace_if/composition/closure