# OptimizingCpp

[wiki](https://upload.wikimedia.org/wikipedia/commons/7/7d/OptimizingCpp.pdf)

## Misc
* qsort -> std::sort/bsearch -> std::lower_bound
* function objects are more easily to expanded inline, while function pointers are rarely inlined
* group arguments into struct -> beauty and register based passing argument
* virtual function inside template class will copied everywhere if realization is based on vtable/RTTI data
* Define volatile only those variables that are changed asynchronously by hardware devices.
* Use initializations instead of assignments, through assignments may be implicit direct ctor
* Member function templates cannot be declared virtual. This constraint is imposed because the usual implementation of the virtual function call mechanism uses a fixed-size table with one entry per virtual function. However, the number of instantiations of a member function template is not fixed until the entire program has been translated. Hence, supporting virtual member function templates would require support for a whole new kind of mechanism in C++ compilers and linkers. In contrast, the ordinary members of class templates can be virtual because their number is fixed when a class is instantiated
* How the compiler handles arrays that are variable length is that in this case it did math on the stack pointer in the amount of the size of the array, basically allocating the dynamic array on the stack as you would expect. For the static sized array the math done on the stack was a static number not a passed in parameter.
* alloca is not good practice
* + [ref](https://stackoverflow.com/questions/1018853/why-is-the-use-of-alloca-not-considered-good-practice)
* appending
* + single -> push_back
* + sequence -> insert
* + STL Alg -> back_inserter
* In C language and also in C++, such variables and functions may be declared static. Though, in modern C++, the use of static global variables and functions is not recommended, and should be replaced by variables and functions declared in an anonymous namespace.
* + [ref](https://stackoverflow.com/questions/4422507/superiority-of-unnamed-namespace-over-static)
* N-place cache
* + static map
* small range sort -> counting sort O(N+M)
* first N -> partial_sort/nth_element
* To empty a vector<T> x object without deallocating its memory, use the statement `x.resize(0);`
* To empty it and deallocate its memory, use the statement `vector<T>().swap(x);`
* For every copyable concrete class T which, directly or indirectly, owns some dynamic memory, redefine the appropriate swap functions.
* If you have to perform boolean operations on a set of bits, put those bits in an unsigned int object, and use bitwise operators on it.
* + The bitwise operators (&, |, ˆ, <<, and >>) are translated in single fast instructions, and operate on all the bits of a register in a single instruction.
* Exploit non-standard routines to round floating point numbers to integer numbers.
```c++
#if defined(__unix__) || defined(__GNUC__)
    // For 32-bit Linux, with Gnu/AT&T syntax
    __asm ("fldl %1 \n fistpl %0 " : "=m"(n) : "m"(x) : "memory" );
#else
    // For 32-bit Windows, with Intel/MASM syntax
    __asm fld qword ptr x;
    __asm fistp dword ptr n;
#endif 
```
* 