# Ideas

## Match cases
Similar to switch statements, but includes a way to check multiple properties/values at the same time
```via
match value {
    case [x, y] when x > y { print("X is greater") }
    case [x, y] when x < y { print("Y is greater") }
    case [x, y] when x == y { print("Both are equal") }

    default {
        print("Values are equal")
    }
}
```

## Lua/C++ interop
Add a way to import Lua/C++ code
- Parse the Lua code with the vLua parser (custom parser for interpreting Lua code as via code)
- For C++, first compile into assembly and then run with LLVM during runtime

Syntax:
```via
import "module.lua" as module_lua
import "module.cpp" as module_cpp
```

```via
Lua { ... }
Cpp { ... }
```

## Meta programming
Add macros, definitions, compile-time control
- Macros similar to Rust (using the `macro` keyword and denoted with ! at the end of the identifier)
- Definitions with the `define` keyword
- Add a custom type that the compiler can use to operate during compile time (similar to `constexpr(<T>)` and `consteval(<T>)`)
- Use $ to denote compile time operations, eg. replace `$defined(has_printf)` with true during compile time if it is defined

Syntax:
```via
macro printf(__fmt, ...) {
    print(string.format(__fmt, ...))
}

define has_printf: true

if $defined(has_printf) {
    printf!("Hello, %s!", "World")
}

if $defined(has_printf) and $has_printf == true {
    printf("printf is defined and set to true! %s", "this statement was emplaced by the compiler!")
}
```

## Error handling
Add a custom error object that contains an error message and a stacktrace to see where it occurs
Syntax:
```via
try { ... }
catch(Error e) {
    print(string.format("Error: %s, StackTrace: %s", e.error, e.stacktrace))
}
```

## Compile-time attributes
Add attributes for declarations such as functions, structs and namespaces
Some possible attributes:
- `@optimize(level)` Optimizes the declaration during compile time
- `@inline` Inlines the declaration during compile time (replaces every instance of it)
- `@cache` Caches the result of a function, similar to `@lru_cache` in Python

## Improved function chaining
Add a function stream operator that can call functions in a chain
Syntax:
```via
local result = do_something() >: do_something_else >: do_final
```
This will call do_something with the "initial arguments of the function stream chain" and will set result to the last operations return
value in the chain. It will pass the return values of a function to the arguments of the next function in the chain

## Type traits
Add type traits similar to Rust. A type trait is well, a trait that a complex type can have
Syntax:
```via
trait Printable {
    print: (String) -> void
}

trait HasName {
    name: String
}

type Person {
    print: (String) -> void,
    name: String
} is Printable and HasName

if (types.hastrait<Person, HasName>()) {
    print("Person has a name!")
}
```
If the useage of a trait is incorrect, the compiler will throw an error

## References
Add a new keyword `ref` to avoid using `memory.pointer` to pass references around
Syntax:
```via
func increment(ref n: Number) {
    n++
}
```
This would update `n` globally

