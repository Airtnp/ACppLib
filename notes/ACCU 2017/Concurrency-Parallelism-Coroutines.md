# Concurrency-Parallelism-Coroutine

[video](https://www.youtube.com/watch?v=UhrIKqDADX8&list=PL9hrFapz4dsMQuBQTnHXogJpMj6L_EQ10&index=5)


## Parallelism
* overloads of `<algorithm>` with an execution policy
* `std::execution::seq`
* + Sequential
* + No additional thread-safety requirements
* `std::execution::par`
* + Ingeterminately sequenced execution on unspecified threads
* + Apply operations on separate objects must be thread-safe
* `std::execution::par_unseq`
* + unsequenced execution on unspecififed threads
* + Operations must be thread-safe and not need any synchronization; may be interleaved, and may switch threads
* Throwing an exception will call `std::terminate`

## Coroutine
* Coroutine: a function can be suspended mid execution and resumed at a later time
* Stackful : entire call stack is saved
* Stackless: only the locals for the current function are saved
* + everything localized
* + minimal memory allocation
* + coroutine overhead can be eliminated by the compiler
* + Can-only susoend coroutines (using `co_await` means the current function must be a coroutine)
* + Can-only suspend current function (suspension returns to caller rather than suspending caller too)
* Coroutine TS: stackless coroutines
* + contain one expression of `co_await/co_yield/co_return`
* + return a type with corresponding coroutine promise
* + `co_return` some value : return a final value from the coroutine
* + `co_await` some awaitable: suspend this coroutine if the awaitable expression is not ready
* + `co_yield` some value: return an intermediate value from the coroutine; the coroutine can be reentered at the next statement
* coroutine promise
* + a class that handles creating the return value object from a coroutine and suspending the coroutine

## Concurrency
* Continuations for futures
* + `std::exp::future` 
* + - `shared_future`
* + - `then`
* + - `promise`
* + - `when_any`/`when_all`
* + Continuation is a new task to run when a future becomes ready
* + Continuations are added with the new `then` member function
* + Continuation functions must take a `stdexp::future` as the only parameter
* + the source future is no longer `valid()`
* Waiting for one or all of a set of futures
* Latches & Barriers
* Atomic Smart Pointer

## Coroutines and Continuations

## Executors
* `context`
* `execute(f)` member function
* `execute(e, f)` free function
* `post(e, f)` queue f for execution with e ASAP, without blocking current task
* `defer(e, f)` If currently running a task on e, queue f for execution with e after current task has finished. Otherwise, same as `post(e, f)`
* `sync_execute(e, f)`
* `async_post(e, f)`
* `async_defer(e, f)`
* with `<algorithm>` extended `std::executor`