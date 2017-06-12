# Postmodern Immutable Data Structure

[video](https://www.youtube.com/watch?v=ZsryQp0UAC8&index=18&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)

## Copying problem
* undo history
* persistence
* + `std::async([=]{ save(d); });`
* thread
* mutabilit => (pass by value === copying)

## Immutable data structures
* all methods marked const
* persistent data structure
* + old values are preserved
* structural sharing
* + no copying
* + compact history
* + fast comparison

## Immer
* [immer](https://sinusoid.es/immer/)
* idiomatic
* + C++ is not Haskell
* performant
* + cache-efficient
* customizable
* + stackable allocator (local)
* + optional GC
* + thread-safety
* data as nodes
* + persistence graph history
* Chris Okasaki: Pure Functional Data Structures
* Ralf Hinze/Ross Paterson: Finger Trees: A general-purpose Data structure
* Phil Bagwell: Array Mapped Tries/RRB-Tress Efficient immutable vectors
* Rich Hickey: Value, Identity and State
* Vector
* + pointer to thunks
* + radix balanced tree
* + M = 2^B
* + - in practice (M = 32, B = 5) cache-friendly
* Radix balanced search
* + v[17] -> 010001 -> 0001 (by 01) -> 01 (by 00)
* If we `v.set(17, 'R')`
* + create copy of layers of tree and shared them rest of them
* Operations
* + effective O(1)
* + - random access
* + - update
* + - push back
* + - slice right
* + O(n)
* + - insert
* + - concat
* + - push_front
* + - slice left
* relaxed radix balanced tree
* + slice left is now O(1)
* + by add tag on parents to mark the amount of nodes
* embedding radix balanced tree
* + two branching factors
```c++
vector<int> myiota(vector<int> v, int first, int last) {
    auto t = v.transient();
    std::generate_n(
        std::back_inserter(t), last - first, [&] { return first++; }
    );
    // or v = std::move(v).push_back(i);
    return t.persistent();
}

```
* dirty marker
* action -> dispatcher -> store -> view