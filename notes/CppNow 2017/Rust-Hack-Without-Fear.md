# Rust: Hack Without Fear

[video](https://www.youtube.com/watch?v=lO1z-7cuRYI&index=1&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)

## What Rust has to offer
* Stong safety guarantees
* + no seg-faults/data-races/expressive type system
* Without compromising on performance
* + no gc/no runtime

## Intro
```rust
struct Event {
    key: u32,
    cout: u32,
    name: InternedString // (error: the type `InternedString` does not implement `Send`)
}

struct InternedString {
    value: Rc<String>  // (non-atomically reference-counted string)
}
```

## Rust the language

### Zero-cost Abstraction
```rust
fn is_whitespace(test: &str) -> bool {
    text.char()
        .all(|c| c.is_whitespace()) // capture like Ruby
}

fn load_images(paths: &[PathBuf]) -> Vec<Image> {
    paths.par_iter()
        .map(|path| Image::load(path))
        .collect()
}
```

### Memory safety
```c++
void example() {
    vector<string> vec; // stack and inline layout
    auto& elem = vec[0]; // lightweight reference
    vec.push_back(some_string);
    cout << elem;       // Error! Vector may rearrange the elements, may be dangling pointer!
                        // deterministic destruction
}
```
* The c++ code mutating the vec freed old contents and do aliasing (more than one pointer) to memory
* Ownership and Borrowing
* + if mutatable, then no aliasing
* Owned (default move)
* + in Rust, `clone()` is explicit
* + in Rust, rvalue reference enforced at compilation time
```rust
fn publish(book: Vec<String>) { // Take the ownership of the vector
    // only access of book
    // If in cpp, default copy
}

fn main() {
    let mut book = Vec::new();
    book.push(...);
    book.push(...);
    publish(book); // Give ownership
    publish(book); // Error: use of moved value `book`
}
```
* Shared reference
```rust
fn publish(book: &Vec<String>) { // Change type to reference

}

fn main() {
    let mut book = Vec::new();
    book.push(...);
    book.push(...);
    publish(&book); // Borrow the vector, creating a reference
    publish(&book);
    // book mutable here
    {
        let r = &book; // book borrowed here
        book.push(...); // cannot mutate while shared
        r.push(...); // cannot mutate through reference
    } 
    // borrow ends, book is mutabtle again.
    book.push(...);
}
```
* back to first example
```rust
fn example() {
    let mut vector = Vec::new();
    //...
    let elem: &String = &vector[0]; // vector borrow here
    vector.push(some_string); // cannot mutate here
}
```
* mutable reference
```rust
fn edit(book: &mut Vec<String>) { // Mutable reference
    book.push(...);
}

fn main() {
    let mut book = Vec::new();
    book.push(...);
    book.push(...);
    edit(&mut book); // mutable borrow
    edit(&mut book);
    // book mutable here
    {
        let r = &mut book;  // book borrowed here
        book.len();         // cannot access while borrowed
        r.push(...);        // &mut ref can mutate
    }
    // borrow ended, accessible again
    book.push(...);
}
```
* Lifetime of a reference
* + lifetime is part of type system
* + `let r: &String = &book;` -> `let r: &'l String = &book;` (l is scope lifetime)
* + [ref](http://blog.csdn.net/renhuailin/article/details/46471233)
```rust
// or just fn first(v: &Vec<String>) -> &String
fn first<'a>(v: &'a Vec<String>) => &'a String {  // named lifetime (lifetime as template parameter)
    // return a reference derived from v
    return &v[0];
}

fn example() {
    let mut book = Vec::new();
    {
        let r = first(&book); // book is borrowed for this bracket-enclosing span
        book.push(...);
    }
}
```


| Type |     Ownership     |Alias? |Mutate?|
|:----:|:-----------------:|:-----:|:-----:|
| T    | Owned             |   F   |   T   |
| &T   | Shared reference  |   T   |   F   |
|&mut T| Mutable refernece |   F   |   T   |
|:----:|:-----------------:|:-----:|:-----:|


### Traits
```rust
// Implemented for a given type
// Interface with one method
trait Clone {
    fn clone(&self) -> Self; // Method that borrows its receiver (Self is reserved)
}

// T is cloneable (typeclass)
impl<T: Clone> Clone for Vec<T> {  // implementation for vectors
    fn clone(&self) -> Vec<T> {
        let mut v = Vec::new();  // create
        for elem in self {  // iterate (using reference)
            v.push(elem.clone()); // push clone
        }
        return v;  // return 'v'
    }
}
// impl<T: Noned + Copy + Eq + PartialEq> Eq for Optioned<T> {}
```
* Marker traits
* + no members
* + `trait Send {}` safe to send between threads (thread-safe)
* + - T: String/u32/Arc<String>... (atomically Arc)
* + - F: Rc<String> 
* + `trait Copy {}` safe to memcpy
* + - T: u32/f32
* + - F: String/Rc<String>
* Coherent rules
* + non-overlapping(?)

### Parallelism
* lib-based concurrency
* multi-paradigm
* data race
* + sharing + mutation + no-ordering
```rust
fn qsort(vec: &mut [i32]) {
    if vec.len() <= 1 { return; }
    let pivot = vec[random(vec.len())];
    let mid = vec.partition(vec, pivot);
    let (less, greater) /*: &mut [i32]*/ = vec.split_at_mut(mid); // same lifetime, no data race
    rayon::join(|| qsort(less)
                || qsort(greater));
}
// fn split_at_mut<'a>(v: &'a mut [T], ...) -> (&'a mut [T], &'a mut [T])
```

|    Paradigm      |     Ownership?     | Borrowing? |
|:----------------:|:------------------:|:----------:|
|    Fork-join     |         F          |     T      |
| Message-passing  |         T          |     F      |
|     Locking      |         T          |     T      |
|     Lock-free    |         T          |     T      |
|     Futures      |         T          |     T      |
|:----------------:|:------------------:|:----------:|


### Unsafe
* Ownership/borrowing/traits give tools to enforce safe abstraction boundaries.
```rust
fn split_at_mut(...) {
    // Trust me
    unsafe {
        //...
    }
}

```
* Rust compiler (~350k lines) 4% unsafe (LLVM-binding)
* + FFI

* RFC -> Nightly -> Beta -> Stable