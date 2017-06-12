# Composite Arithmetic Types Are > the + of Their Parts

[video](https://www.youtube.com/watch?v=1xSyUoYgSR4&index=17&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)

## Pitch
* is_composite
* + composed from fundamental arithmetic types
* + substituted for fundamental arithmetic type
* + can be used to compose other arithmetic types
* + separation of concerns
* ensure no overflow/intermediate on operations
```c++
template <typename T>
class safe_integer {
public:
    //...
private:
    T rep_;
};
```
* can be extended
```c++
template <std::size_t N, typename T>
class elastic_integer {
    //...
private:
    T rep_;
};
```
* `safe_elastic_integer`/`fixed_pointer`/`precise_integer`
* combine them by template arguments