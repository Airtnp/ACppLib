# The C++ Type System Is Your Friend

[video](https://www.youtube.com/watch?v=MCiVdu7gScs)

## Overall
* Safe/Performant/Reusable
* + prevent avoidable mistakes in compiler-time
* + zero run-time cost compared to acceptable compile-time cost
* + reusable/generic
* Typeless programming
* + Assembler
* + - Integer <-> address
* + B/BCPL
* + - 3 * (4 + 5) = 27
* + - 3 (4 + 5) = func(9) (&func = 3)
* + C preprocessor
* + - programming with strings
* Machine-typed programming
* + C/Primitive-based Cpp
* + - underlying machine types (int/float/typed pointer)
* + - strucures/aggregates
* + - avoid type puns and mistabkes of assembler
* Type-rich programming
* + Higher-level C++
* + - type-system -> lightweight abstraction
* + - proof-system (compiler-time checking if a construct is illegal)
| Language | Run-time | Compiler-time |
| :------: | :------: | :-----------: |
| C++      | Machine-types | Language-types/Application-types |
| C        | Machine-types | Language-types |
| ASM      | Machine-types | |

* Primitive or typed API
* + replace `Date(int, int, int)` to `Date(Year, Month, Day)` by `using` (type aliasing)
* + Completely separate type `class Day {}`
* Physical types
* Whole Value pattern
```c++
class T {
    explicit T(U);
    operator U() const;
}

template <UnitType U>
class Unit {
    explicit Unit(T);
    //...
}
```
* + hold-value without no operation (operation done on the base type)
* + adding checking of values (`constexpr`)
* + add specific operation
* + CRTP (`class Year : public Ordered<Year>`)
* + ref: Modern Cpp Design (Template parameters as units)
* + - `template <value_t, Unit_sys, mass_t, length_t, time_t>`
* + checker (Concept)