# Haskell taketh away

[video](https://www.youtube.com/watch?v=lC5UWG5N8oY&index=11&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)

## History & Context
* against impure strict functional langs (ML, Scheme)
* pure functional
* side effects (change type)
* composing first-class actions `(Vector(IO ()))`
```Haskell
data Expr
    = Var Var
    | Lit Literal
    | App Expr Expr
    | Lam Var Expr
    | Let Bind Expr
    | Case Expr Var Type [(AltCon, [Var], Expr)]
    | Cast Expr Coercion
    | Type Type
    | Coercion Coercion
data Bind = NonRec Var Expr | Rec [(Var, Expr)]
data AltCon = DEFAULT | LitAlt Lit | DataAlt DataCon
```
| Comparisons            | C++ | SML | Haskell | Eiffel | Java Generics |
|------------------------|-----|-----|---------|--------|---------------|
| Multi-type concepts    | -   | +   | +       | -      | -             |
| Multiple constraints   | -   | +-  | +       | -      | +             |
| Associated type access | +   | +   | +-      | +-     | +-            |
| Retroactive modeling   | -   | +   | +       | -      | -             |
| Type aliases           | +   | +   | +       | -      | -             |
| Separate compilation   | -   | +   | +       | +      | +             |
| Implicit instantiation | +   | -   | +       | -      | +             |
| Concise syntax         | +   | +-  | +       | -      | +-            |
* type families (& associated types)
* type classes (concept, traits)
* lambdas, map/reduce programmings
* garbage collection
* purity/controlled effects/type-effect systems
* formal verification
* Intel CnC
* + Side effects :: IO ()
* + Compute Steps :: CnC () / CnC Monad
* Facebook
* + Haxl () / Haxl Monad


## Taxonomy of side effects
* IO () 
* + mutate mem
* + syscalls
* + FFI
* + anything
* ST s ()
* + thread-local
* + mutable state
* + `runST :: (forall s, ST s t) -> t`
* + `sort' :: STVector s Int -> ST s ()`
* + `sort :: Vector Int -> Vector Int`
* STM ()
* + Software transactional memory
* Heap
* Immutable data
* + ST (thread-local state)
* + - Disjoint-shared ST state
* + STM (transactional state)
* + - Monotonic object
* + IO (global shared state)
```haskell
-- IO
do r <- newIORef
   writeIORef r 3
   readIORef r

-- ST
do r <- newSTRef
   writeSTRef r 3
   readSTRef r
```
* SC-Haskell
* IO global shared state
* + shared mutable data, but protected by a lock ST
* + - Not `IORef Int`, but `(Lock s, STRef s Int)`
* + - `withLock :: Lock s -> ST s a -> IO a`
* sequential consistency
* race-freedom
* determinism
* memory safety
* Safe Haskell
* + Safe
* + Unsafe
* + Trustworthy




## Race-freedom & Determinism
* System-level deterministic parallelism
* `main :: DetIO ()` restricted side-effects
* + f(in-files) = out-files
* deterministic parallel shell scripts
* `forEach dir \file ->:` static (parallel)
* `system("gcc -c " ++ file)` dynamic (sequential)
* `foreighBinary :: StdinStrm -> StdoutStrm`
* DetIO
* + Noninterference (non-write same memory)
* + - purely functional flavor: no problem
* + - imperative flavor (disjoint regions, PLDI'14, preserve alias freedom)
* + - - alias free
* + - - `newArr 7` -> `duplicate` -> `Split 3` (in-place parallel sorting in a pure function)
```haskell
do 
    newArr 7
    v1 <- ask
    fill v1 1
    forkJoin (Split 3)
        (do v2 <- ask; fill v2 2)
        (do v3 <- ask; fill v3 3)
```
* + Reduction instead of Noninterference
* + - LVars (POPL'14) LVish library (lock-free CDS)
* + - - commuting inserts
* + - - blocking read
* + - - freeze operation
* + - - traverse frozen3



## Performance & Data Representation
* pros
* + unbox primitive types in records (newtype)
* + unbox records in records
* + vectors with unboxed elements
* + automatic AOS -> SOA in Vector
* + full set of numeric types & unaligned operation (bit twiddling ready)
* + fuse traversals, eliminate temp data
* cons
* + unwanted laziness, thunks
* + unpredictable deforestation
* + performance
* + - large RTS
* + - not-so-scalable GC
* + - easy for beginner/intermediate programmers to write very slow code
* + - hard to mentally model what the compiler and RTS can optimize
* Data Baking
* + using external representation as internal memory representation
* + C++: technique used in game industry
* + Haskell: as of GHC 8.2
* + - leverage immutable and monotonic data -> compact region (ICFP'15A)
* + - block structured heap, internal = external rep. No type system reqmts
* Dense tree
```c++
enum Type { Leaf, Node };
struct Tree {
    enum Type tag;
    union {
        struct { long elem; };
        struct { struct Tree* l; struct Tree* r; };
    };
};
```
```Haskell
data Tree = Leaf Int64 | Node Tree Tree
```
* formal verification (full correctness)
* + traditional
* + - verify some properies in Haskell type system
* + - - do proofs in Agda or Coq
* + - - extract to Haskell, link against existing code
* Haskell Done
* + Type-level programming
* + Template metaprogramming
* + Multicore runtime
* + SIMD intrinsics
* + Futures, working stealing
* + Parallel GC
* + Green threads, multicore IO mgr
* + Rewrite-optimizations in libraries
* + Type introspection and extensible deriving
* + Lazy evaluation
* + Unboxed records
* Haskell Future
* + Dependent Type
* + Refinement Types (Liquid Haskell Prover)
* + Linear Types (unique typing) (GC-free Haskell)
* + Better strictness support (lazy -> hybrid)
* + Better data representation
* + ML-style modules (different compilation units)
* Haskell Undecided
* + Concurrent GC
* + Auto-vectorization
* C++ and Haskell EDSLs
* + Halide
* + ML EDSLs (often Python-embedded)
* + statically typed host languages (C++, Scala, Haskell) have embedding advantages
* + - overloading tricks
* + - deferred expressions
* + - compilation checking
* C++ and Haskell commonalities
* + big complicated
* + old but changing
* + good for generic
* + good for data baking
* + good host for EDSLs
* Haskell steal from C++
* + link time deduplication of specializations
* + More programming control over abstraction cost (eg. specialization and dictionaries)
* 
