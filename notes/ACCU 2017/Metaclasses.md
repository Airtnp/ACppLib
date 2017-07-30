# Thoughts on Metaclasses

[video](https://www.youtube.com/watch?v=6nsyX37nsRs)

## Overview
* enable a new kind of efficient (compile-time) user-defined abstraction
* + custom trans from source to ordinary class def
* + user-defined named subset of classes with common characteristics
* Building on/with
* + published TS: concepts, constexpr, if constexpr
* + reflection
* + compile-time(meta) programming
* Reflection
* + `$T, $expr`
* Compile-time programming
* + `constexpr { for (auto m: $T.variables()) if (m.name() == ) ->  }`
* Source-code ----compiler-----> AST
* hook the compiler

## Metaclasses
* $class denotes a metaclass
* + `$class interface {...};`
* + more specific than class
* Typical uses
* + enforce rules
* + provide defaults
* + provide implicitly
* Typical classes
* + ref: p0707r0/3
* + ordered/plain_struct/copyable_pointer/bitfield/safe_union/namespace_class

```c++
// implementation of interface

#include <cppx/meta>
#include <cppx/compiler>

using namespace cppx::meta;

$class interface {
    ~interface() noexcept {}
    constexpr {
        compiler.require($interface.variables().empty(), 
            "interfaces may not contain data members");
        for/*pre-kona version ...*/ (auto f: $interface.functions()) {
            compiler.require(!f.is_copy() && !f.is_move(),
            "interface may not copy or move; consider a virtual clone");
            if (!f.has_access()) // access specifier
                f.make_public();
            compiler.require(f.is_public(), "interface functions must be public");
            f.make_pure_virtual();
        }
    }
};

```

```c++

$class basic_value {
    basic_value() = default;
    basic_value(const basic_value&) = default;
    basic_value(basic_value&&) = default;
    basic_value& operator=(const basic_value&) = default;
    basic_value& operator=(basic_value&&) = default;

    constexpr {
        for (auto f: $basic_value.variables())
            if (!f.has_access())
                f.make_private();
        for (auto f: $basic_value.functions()) {
            if (!f.has_access())
                f.make_public();
            compiler.require(!f.is_protected(), "a value type may not have a protected function");
            compiler.require(!f.is_virtual(), "a value type may not have a virtual function");
            compiler.require(!f.is_destructor() || f.is_public(), "a value destructor must be public");   
        }
    }
};

// ordered: operator<>=!...
$class value : basic_value, ordered {};

```

```c++

$class basic_enum : value {
    constexpr {
            compiler.require($basic_enum.variables.size() > 0, "an enum cannot be empty");
            if ($basic_enum.variables.front().type().is_auto())
                -> { using U = int; }
            else -> { using U = $basic_enum.variables.front().type(); }
            for (auto o: $basic_enum.variables) {
                if (!o.has_access()) o.make_public();
                if (!o.has_storage()) o.make_constexpr();
                if (!o.has_has_auto_type()) o.set_type(U);
                compiler.require(o.is_public(), "enumerators must be public");
                compiler.require(o.is_constexpr(), "enumerators must be constexpr");
                compiler.require(o.type() == U, "enumerators must use same type");
            }
            -> { U$ value; }  // instance value
    }
}

// binary codes, like win-style
$class flag_enum : basic_enum {
    flag_enum operator& (const flag_enum& that) { return value & that.value }
    // ...
    explicit operator bool() { value != none; }
    constexpr {
        compiler.require(objects.size() <= 8*sizeof(U), "there are " + objects.size() + " enumerators but only room for " + to_string(8*sizeof(U)) + " bits in value type " + $U.name());
        compiler.require(!numeric_limits<U>.is_signed, "a flag_enum value type must be unsigned");
        U next_value = 1;
        for (auto o: $flag_enum.variables) {
            compiler.require(!o.has_default_value(), "flag_enum enumerator values are generated and cannot be specified explicitly");
            o.set_default_value(next_value);
            next_value *= 2;
        }
    }
    U none = 0;
};


```

```c++


template <basic_enum E>
auto to_string(E value) {
    switch(value) {
        constexpr {
            for (auto o: $E.objects)
                if (!o.default_value.empty())
                    -> { case o.default_value()$: return E::(o.name()$); }
        }
    }
}

```

```c++

$class bitfield : final, comparable_value { // no derivation
    constexpr {
        auto objects = bitfield.variables(); // take a copy of the class’s objects
        size_t size = 0; // first, calculate the required size
        for (auto o : objects) {
            size += (o.bit_length == default ? o.type.size*CHAR_BITS : o.bit_length;
            if (!o.has_storage()) o.make_member();
                compiler.require(o.is_member(), "bitfield members must not be static");
            compiler.require(is_trivially_copyable_v<o.T>, "bitfield members must be trivially copyable");
            compiler.require(!o.name.empty() || o.T == $void, "unnamed bitfield members must have type void");
            compiler.require(o.type != $void || o.name.empty(), "void bitfield members must have an empty name");
            if (o.type != $void) -> { // generate accessors for non-empty members
                o.T$ o.name$ () { return /*bits of this member cast to T*/; }
                set_(o.name)$(const o.T$& val) { /*bits of this value*/ = val; }
            }
        }
    }
    
    $bitfield.variables().clear(); // now replace the previous instance vars
    byte data[ (size/CHAR_BITS) + 1 ]; // now allocate that much storage

    bitfield() { // default ctor inits each non-pad member
        constexpr {
        for (const auto& o : objects)
            if (o.type != $void)
                -> { new (&data[0]) o.type.name$(); };
        }
    }

    ~bitfield() { // cleanup goes here
        constexpr {
            for (auto o : objects)
                if (o.type != $void)
                    -> { o.name$.~(o.type.name$)(); }
        }
    }

    bitfield(const bitfield& that) : bitfield() { // copy constructor
        *this = that; // just delegate to default ctor + copy =
    } // you could also directly init each member by generating a mem-init-list

bitfield& operator=(const bitfield& that) { // copy assignment operator
    constexpr {
        for (auto o : objects) // copy each non-pad member
            if (o.type != $void) // via its accessor
                -> { case o.num$: set_(o.name$)() = that.(o.name)$(); }
    }
}

bool operator==(const bitfield& that) const {
    constexpr { // (we’ll get != from ‘comparable_value’)
        for (auto o : objects) // just compare each member
            -> { if (o.name$() != that.(o.name)$()) return false; }
        return true;
    }
}

```


```c++

template<class T>
$class property<T> {
    // ...
};

```

### interface

```c++

class Shape {
public:
    virtual int area() const = 0;
    virtual void scale_by(double factor) = 0;
    // ...
    virtual ~Shape() noexcept {}
};

interface Shape {
    int area() const;
    void scale_by(double factor);
    pair<int, int> get_extents() const;
    // ...
};

constexpr {
    compiler.debug($Shape);
}

```
* default+enforce: all public pure virtual functions
* enforce: no data members, no copy/move


### value

```c++

class Point {
    int x = 0, y = 0;
public:
    Point(int, int);
    // ...
    friend bool operator==(const Point& a, const Point& b);
    // ... many operators
};

value Point {
    int x = 0, y = 0;
    Point(int, int);
};


```

* default+enforec: copy/move, comparisons, default ctor
* default (opt) : private data, public function
* enforce: no virtual functions

### literal_value

```c++

template <class T1, class T2>
literal value pair {
    T1 first;
    T2 second;
};

```

* default+enforce: copy/move, comparison, default ctor, explicit ctor, constexpr, make_*, piecewise_construct, usings

### enum

```c++

flag_enum openmode {
    auto in, out, binary, ate, app, trunc;
};

openmode mode = openmode::in | openmode::out;

```

### property

```c++

class MyClass {
    property<int> value {
        string val;
        void set(int v) { val = to_string(v); }
        int get() const { return stoi(val); }
    };
};

```

* default+enforce: private data, public functions
* enforce: no data members, no copy/move
* generate: value, get, and set

## Extension
* Qt moc (moc compiler -> generate moc_*.cpp)
* Qt extension

```c++

class MyClass : public QObject {
    Q_OBJECT
public:
    ...
    Q_PROPERTY(int value READ get_value WRITE set_value);
signals:
    void mySignal();
public slots:
    void mySlot();
};

QClass MyClass {
    property<int> value {}; // default
    signal mySignal(); // deduction guide(1z) to get default void
    slot mySlot();
};

```

* C++/CX (for WinRT)
* + IDL (like COM) 
* + - idl -> MIDL -> generate _i.c _p.c -> .winmd

```c++

[
object,
uuid(....),
]

interface IFoo : IInspectable {
    [propget]
    HRESULT Get {
        [in] UINT keym
        [out, retval] SomeClass** value
    };
    [propout]
    HRESULT Set {
        // ...
    };
};

rt_interface IFoo {
    constexpr string uuid = "...";
    property<SomeClass> {
        SomeClass Get(uint32_t key);
        void Set(uint32_t key, const SomeClass& v);
    }
};


```