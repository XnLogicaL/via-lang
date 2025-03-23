# Data Types

via has a variety of built-in data types. These types include:

- `Nil`
- `Number`
- `Bool`
- `String`
- `Table`
- `Function`

Note: Type annotations in via are optional. All the given examples utilize type annotations for clarity purposes.

## Nil type

The `Nil` type represents an empty value. It doesn't actually carry any data, and cannot be garbage collected. It's use cases are simple, but very crucial.

Declaraing a variable with `nil`:
```lua
local val: Nil
local val2: Nil = nil
```

Both of these statements are valid, and will not produce a compiler error.

Comparing nil values will **always** result in `true`. 
```lua
local equal = nil == nil ## true
```

## Number type

The `Number` type provides a unified interface for all numeric values, represented as a 64-bit floating-point number under the hood.

Declaring a variable with `Number`:
```lua
local val: Number
```

Declaring a variable with an uninitialized number is not recommended, is considered bad practice, and will be greeted with a warning. The default inferred value is `0`.

### Supported numeric literals
via supports decimal, hexadecimal, and binary literals:
```lua
local val: Number = 15
local val: Number = 0xE ## Automatically converted to 15
local val: Number = 0b1011 ## Automatically converted to 15
```
They are automatically converted into a floating point representation during compilation.

## Bool Type

The `Bool` type represents a binary state: `true` or `false`. It can also be cast to a `Number` when needed.

Declaring a variable with `Bool`:
```lua
local val: Bool
```
Boolean values do not have a default value. Uninitialized boolean variables are invalid. This statement will result in a **compiler error**.

The reserved pseudo-keywords `true` and `false` are used to assign boolean values.
```lua
local val: Bool = true
local val2: Bool = false
```

## String type

The `String` type is a data type used to represent string literals. It is automatically managed and doesn't have a definite size_t.

Declaring a variable with `String`:
```lua
local val: String = "Hello"
```

As of via 0.x.x, string interpolation is **not supported**. `string.format` can be used as a placeholder. 

## Table type

The `Table` type is a highly dynamic container type that can be used as an array, dictionary or map. It can be indexed with any key which is either a `Number`, a `string`, or has a `__to_string` method for hashing. 

Declaring a variable with `Table`:
```lua
local val = {}
local arr = {1, 2, 3, 4}
local dict = {x = 1, y = 2, z = 3}
```
Now all of these variables technically hold a `Table` value.

## Function type

