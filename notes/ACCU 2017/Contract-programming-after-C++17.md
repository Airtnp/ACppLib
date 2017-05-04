# Contracts programming after C++17

[video](https://www.youtube.com/watch?v=IBas3S2HtdU&list=PL9hrFapz4dsNYgaDy3CL76Cz2J_jSZrHA&index=8)

## Introduction
* Correctness
* Robustness
* More than run-ime
* contracts -> optimizing (like `restrict`)
* semantics to external tools (like type hinting in Python)
* avoid comment/code synch issue

## Contract
* A contract is the set of 
* + preconditions
* + - expectation `expects`
* + - `double sqrt(double x) [[expects: x>0]]`
* + postconditions
* + - ensurance `ensures`
* + - `double sqrt(double x) [[expects: x>0]] [[ensures result: result >= 0]]`
* + - `[[ensureres r: r >= 0]]`
* + assertions
* + - `[[assert: x >= 0]]` attribute vs. `static_assert/assert`
* + - language feature vs. function
* Effect of contracts
* + no observable effect on a correct program (except performance)
* Contract level
* + `default/audit/axiom`
* + default
* + - cost of checking is expected to be small compared to function execution
* + audit
* + - expected to be used in case where cost of a run-time check is assumed to be large compared to function execution
* + - `bool binary_search(...) [[expects audit: is_sorted(f, l)]];`
* + axiom
* + - expected to be never run=time performed
* + - not evaluated (may contian calls to declared but undefined functions)
* Build level
* + off/default/audit
* Check in sequence
* + side effect
* Violation handlers
* + `void (const std::contract_violation &)`
* Future
* + gradual introduction of contracts
* + test the contracts themselves
* + plugin management
* Continuation mode/Optimization
* + `[[assert: ptr != nullptr]]; if (ptr != nullptr)` (can be optimized)
* Types
* + function pointer not include contract
* + lambda (capture)
* + class (access control)
* + inheritance (override)
* + overload function