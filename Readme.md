# SuperNaiveCppLib

This is a Cpp library full of super naive wheels and **collected(copied)** wheels (see ref) based on C++1y.

Because it's so naive, no comments and documentation supported. And its code-style is much messy.

No common namespace, because loosely connected.

I strongly recommend Herb Sutter's C++ Coding Standard / C++ API Design , my code is full of junk (however, I strongly appreciate the reference code I copied).

## Wheel List (super naive, all incomplete) (* even having no function)
* sn_Alg: some algorithms
* sn_AOP: a simple AOP frame
* sn_Assist: some general assist template/functions
* sn_Binary*: support binary operation
* sn_Builtin: a class support pointer (pointer_reference/intrusive_pointer/shared_ptr) and reference (reference_counter)
* sn_DB: a wrapper of MySQL C API
* sn_DBP: a MySQL connection pool
* sn_Decimal: a big integer library
* sn_DS: some data structures
* sn_Filesystem: support filesystem-related
* sn_Function: support curry/combine/lazy evaluation of functions | support chain/pipeline operation
* sn_LC: support lambda-calculus / combinators
* sn_LCEncoding: support lambda-calculus encoding (Church/Scott...)
* sn_LINQ: support LINQ
* sn_Log: a log class
* sn_Macro: some macros
* sn_PIC: pi-calculus
* sn_PC: a realization of parser combinator
* sn_PD: parsing derivative (CFG)
* sn_PM: type-level pattern-matching and function restriction (Haskell-style) and runtime pattern-matching
* sn_Range*: support range
* sn_Reflection: two realizations of reflection (magic_get/unnamed_pod_reflect && named_reflect/typeless)
* sn_Regex: RegexD: derivative + NFA(recusive-down) | RegexT: parsec no parser | RegexV: NFA + DFA no parser
* sn_StdStream: add more function with std::iostream
* sn_Stream: support basic/memory/file stream
* sn_String: a class dealing with string
* sn_TC: support type-calculus (+compile-time polynomial addition/multiplication)
* sn_TEST: support testing and contract
* sn_Thread: support thread-related
* sn_Type: support type-erasing types.
* sn_TypeLisp: support TypeList operation | Lisp 7 axioms (cons/car/cdr/cond/quote/atom/eq)


## TODO
* Many TODO


## License

* MIT
