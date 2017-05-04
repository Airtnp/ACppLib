# Metaprogramming in C++14

[video](https://www.youtube.com/watch?v=Oc4enqNH-Mc&list=PL9hrFapz4dsPvV6X9Iw3Yd2_N4NCoJwPh)

* Typelist
* boost::mpl (manipulate types)
* + `using Tys = boost::mpl::vector<int, char>;`
* + `using NV = boost::mpl::remove_if<Tys, ...>`
* boost::fusion (manipulate objects)
* + `auto Tys = boost::fusion::make_vector(1, 2.2f);`
* + `auto NV = boost::fusion::remove_if<std::is_void<boost::mpl::_>>(Tys)`
* boost::hana (represent compile-time entities as objects (not types))
* + a way to represent types as values
* + `auto Tys = boost::hana:tuple_t<int, void>;`
* + `auto NV = boost::hana::remove_if(Tys, hana::traits::is_void);`
* + Parser Combinator
```c++
int main() {
    auto parser = combine_parsers(
        lit('('), parse<int>{},
        lit(','), parse<std::string>{},
        lit(','), parse<double>{},
        lit(')')
    );
    hana::tuple<int, std::string, double> data = parser("(1, foo, 3.3);
}
```
* + - `hana::typeid`
* + dimensional analysis
* + simple event system
* + - `hana::map<hana::pair<Events, std::vector>>`
* + serialization