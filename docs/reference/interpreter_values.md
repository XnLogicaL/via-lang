# Interpreter values

The via interpreter represents all data using two core structures:
- `Value` — the raw, unowned data representing primitives and managed heap objects.
- `ValueRef` — the reference-counted wrapper around a Value that manages ownership and lifetime.

## Value

The `Value` structure is a [discriminated union]() container that does not have any ownership semantics. It is defined as the following:

```cpp
enum ValueKind {
  VK_INT,
  VK_FLOAT,
  VK_BOOLEAN,
  VK_STRING,
  VK_ARRAY,
  VK_DICT,
  VK_FUNCTION,
  VK_USERDATA,
};

struct Value {
  ValueKind kind;
  union {
    int i;
    float f;
    bool b;
    String* s;
    Array* a;
    Dict* d;
    Closure* f;
    Userdata* ud;
  } u;
};
```

## ValueRef

`ValueRef`s are structures that wrap `Value` objects and posses a reference counter in order to manage shared ownership of `Value` objects. This structure is defined as the following:

```cpp
struct ValueRef {
  int rc;
  Value val;
  ValueAllocator* al;
};
```

## Object

All managed objects inside the `Value` structure are based on the `Object` structure. It is an empty pure-virtual structure that is only used for convenient managed type semantics. 
