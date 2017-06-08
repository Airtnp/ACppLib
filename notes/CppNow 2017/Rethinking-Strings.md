# Rethinking Strings

[video](https://www.youtube.com/watch?v=OMbwbXZWtDM&index=4&list=PL_AKIMJc4roXJldxjJGtH8PJb4dY6nN1D)

## Inspiration

### the COW (copy on write string) is dead
* reference counter and pointer
* SSO (small string optimized strings have won) directly stack memory
* at what cost?
* + 8.6%+ in average heap block size
* + 12%+ in number of used heap allocations
* + 21%+ in used heap bytes
* + SSO is only a component of the increase
* + Performance difference unknown
* + Code is optimized for COW
* What gain?
* + Atomic ops are not a bottleneck
* + buy/build a library solution
* + ask for more RAM

### string_view
* have limited use to date
* will ripple virally
* changing everything! (like exception(string_view))
* layout
```c++
class sso_string {
    size_t len;
    char* data;
    size_t cap;
};

class string_view {
    size_t len;
    char* data;
};

class cow_string {
    char* data;
};

struct cow_heap {
    size_t len;
    size_t cap;
    int refcount;
    char data[];  
};

class c_string {
    (strlen)
    char* data;
}
```
* constexpr (compile-time string)
* Unicode (UTF-8)
* 64-bit addressing
* STL2 (concepts/ranges)

### Not alone
* CsString
* QString
* FBString
* fixedString
* QStringView
* codecvt deprecated (P0618)
* text_view (P0244)
* char8_t (P0482)

## Experience

* Constants
* + std::string
* + - overhead
* + static const std::string -> const char[]
* Parameters
* + const std::string& -> const char*
* + viral (everywhere)
* + break COW 
* + strlen
* + overhead
* const string& -> const string_view&
* + viral (everywhere)
* + break COW 
* String Builder
* + concat
* + formatter
* SSO locality
```c++
class Widget {
    vtabl* _vptr;
    int refcount;
    size_t size;
    char* end;
    size_t cap;
};
```
* COW Sharing
* Mutable COW not thread-safe
* Immutable
* Nullable
* Encoding

## Rethinking
* Mindset
* Traits
* + code points
* + code units (char/wchar/char16_t/char32_t) (no 8-bit in standard)
* + - `code_unit<Encoding>*`
* + - data(s) -> encoding::code_unit*
* + - decltype(s)::encoding
* + - decltype(c)::encoding
* Ownership / Mutability
* + view (immutable)
* + shared owner (immutable)
* + unique owner
* + `is_owner_v`
* Ownership transfer
* + unique -> shared
* + mutable -> immutable
* Storage Duration
* + static `constexpr char*`
* + automatic `char[3]`
* + dynamic `unique_ptr<char[]>`(table) (custom allocator)
* Storage Duration Traits
* Nullability
* + not nullable: string_view
* + nullable: char*
* + `is_nullable_v`
* + `write_null(void*)`
* + `is_null(void*)`

## Codes
|           |  Shared Immutable |   Unique Mutable   |
|:---------:|:-----------------:|:------------------:|
| Automatic | constexpr char[N] |    FBString SSO    |
|  Dynamic  |      FBString COW | COW FBString SSO   |
* break into shared and unique COW
|           |  Shared Immutable |   Unique Mutable   |
|:---------:|:-----------------:|:------------------:|
| Automatic |       fixed       |     small SSO      |
|  Dynamic  |      shared       |     unique SSO     |
* [rethinking-string](https://github.com/vmware/rethinking-strings)