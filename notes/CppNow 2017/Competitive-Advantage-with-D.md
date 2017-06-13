# Competitive Advantage with D

[video](https://www.youtube.com/watch?v=vYEKEIpM2zo&index=33&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)

## Advantage
* static if
* Sociomantic
* weka.IO
* CTFE
* vibe.d (network)
* pegged (PEG module)
* compilable
* strongly statically typed with type and attribute inference
* powerful template and other compile-time features
* multi-paradigm system language
* both low/high-level
* compile fast
* execute fast
* readily links with C (and C++)
* RAII/GC/RC/manual
* community
* no preprocessor
* no \0 string
* less implicit conversions
* no octal literals
* crippled comma operator
* no support for legacy
* no lexical ordering of declarations (at module scope)
* 


## GC
* parallel
* not random
* can be disable/collect/banned/profiled
* + @nogc

## Objection
* reference types
* + struct: value
* + class: reference
* no slicing/copying/assignment/moving

## Difference
* void main()
* cleaner syntax
* + to!int(str)
* + str.to!int(); // UFCS!
* + str.to!int;   // no need for empty parentheses
* .init
* no rvalue reference
* immutable (shared, not need to lock)
* move/copy semantics
* + Classes don't have rvalues
* + Struct rvalues are moved
* + Struct lvalues are blitted (bit-level transferred)
* + Post-blit function when needed
* + prvalues, xvalues, glvalues, etc. are not in D vocabulary.
* module system
* C arrays
* static arrays (fixed length array)
* dynamic arrays
* UFCS
* containers
* concurrency/parallelism
* + shared
* + immutable
* + synchronized
* + std.concurrency/parallelism/sync
* + Fiber
* CTFE (compile-time function execution)
* Templates
* + function
* + struct/class
* + eponymous
* + general
* + constraints
* + variadic/compile-time foreach
* + user-defined type as template parameter
* + mixins
* operator overloading
* opDispatch
* AliasSeq
* static foreach
* Traits
* + __traits
* user-defined attributes (UDA)
* Design by introspection (DbI)
```dlang
struct MyHook {
    alias
        onBadCast = Abort.onBadCast,
        onLowerBound = Saturate.onLowerBound,
        onUpperBound = Saturate.onUpperBound,
        onOverflow = Saturate.onOverflow,
        hookOpEquals = Abort.hookOpEquals,
        hookOpCmp = Abort.hookOpCmp;
}
alias MyInt = Checked!(int, MyHook);

ref Checked opUnary(string op)() return
if (op == "++" || op == "--") {
    static if (hasMember!(Hook, "hookOpUnary"))
        hook.hookOpUnary!op(payload);
    else static if (hasMember!(Hook, "onOverflow")) {
        static if (op == "++") {
            if (payload == max.payload)
                payload = hook.onOverflow!"++"(payload);
            else
                ++payload;
        } else {
            if (payload == min.payload)
                payload = hook.onOverflow!"--"(payload);
            else
                --payload;
            }
        } else
            mixin(op ~ "payload;");
    return this;
}
```
* SafeD (@safe)
* unit test
* scope
* 