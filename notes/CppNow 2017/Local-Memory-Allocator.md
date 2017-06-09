# Local Memory Allocator

[video](https://www.youtube.com/watch?v=CVbnjydW5M0&index=7&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)

## Background
* copy allocator
* require more up-front design
* complicates user interfaces
* degrade perforamce
* + no special allocator needed
* + poolar chosen allocator supplied
* main memory
* + CPU <-> Main Memory
* cache memory
* + CPU <=> cache <-> main
* Stack Memory (main) ----------- Dynamic Memory Executable program (static data segment / code segment) (fixed size)
* alloca
* + stack
* malloc/free (<new>)
* + dynamic
* global vs local
* + global
* + - operates on a single ubiquitous region of memory
* + - exists throughout the lifetime of a program
* + - inherently accessible from all parts of a program
* + local
* + - operators on a local sub-region of memory
* + - exist for less than lifetime of a program
* + - supplied for client use via a "reference"
* + - used to free memory unilaterally
|         | Global | Local |
| General | malloc/free new/delete tcmalloc jemalloc | multipool_allocator |
| Special | unsynchronized tcmalloc allocator | alloca monotonic_allocator unsynchronized version of multipool_allocator |
* memory allocator
* + (client-facing interface for) a stateful utility or mechanism that organizes a region (possibly non-contiguous) of computer memory, dispensing and reclaiming authroized access to suitable sub-regions on demand 
```c++
class LocalAllocator : public Noncopyable<LocalAllocator> {
    LocalAllocator();
    void* allocate(std::size_t nbytes);
    void deallocate(void* address);
    void release(); // local allocator only
}

```
* + as stateful utility functions
* + as reference_wrapper template parameters
* + as the address of a pure abstract base class

## Understanding
* supply allocator?
* + default global
* + via base class?
* + wink-out memory?
* Lee-algorithm
* global allocator
* + type parameter/abstract base
* monotonic/multipool/multipool<monotonic>
* + type parameter/abstract base
* + normal destructor/winked-out
* 14 in total
* + AS 11: standard allocator (`new/delete`)
* + AS 12: `std::vector<int, allocator>;` type parameters
* + AS 21: `std::vector<int*>` -> `delete` normal destruction
* + AS 22: `Allocator { virtual allocate = 0; virtual deallocate = 0;};` (pure virtual base)
* + AS 31: Monotonic allocator
```c++
void func() {
    char buffer[1024]; // external buffer
    auto localAllocator(buffer, sizeof(buffer));
    vector(&localAllocator);
}
```
* + AS 32: `local.allocate(...), local.deallcate(...)`
* + AS 41: no deallocate (magically winked-out)
* + AS 51: multipool (each adaptive pool maintains its own free-list)
* N: number of instructions executed (per thread)
* W: number of active (worker) threads
* allocation strategy
* + density of allocator operations
* + variation of allocated sizes
* + locality of accessed memory
* + utilization of allocated memory
* + contention of concurrent allocations

## Benchmark Data...