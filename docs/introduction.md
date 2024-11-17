# Introduction

## What is via?

**via** is a powerful, semi-dynamically typed, multi-paradigm programming language enhanced with Just-In-Time (JIT) compilation. It aims to strike a balance between convenience and performance, combining the ease of use found in languages like Python and Lua with the speed of systems programming languages. via is heavily inspired by the Lua (specifically Luau) programming language for it's flexibility and robust features. However, via is more featureful and has been carefully crafted to ensure good development experience, unlike some features of Lua/Luau that I personally don't like. These features include;

- Abundance of runtime errors
    - via solves this issue by emitting most errors during compile time, so you won't have to wait for your program to fail.
    via still does have runtime errors, just not as many as Lua/Luau
- Unusual syntax
    - Even though the Lua/Luau syntax is great for simplicity, it lacks in functionality. via borrows keywords from Lua and combines
    them with features from more robust languages such as Rust.
- Limited standard library
    - Lua/Luau is quite infamous for it's minimal standard library (which technically isn't even a library), wihch in my opinion deals
    a huge blow to the functionality of the language. via addresses this issue by providing you with the via STL (via standard template library), which includes both abstract high level concepts and low level features that give you extra control over everything.

## Design and Syntax

via employs a curly brace-based syntax, reminiscent of languages such as JavaScript and the C family. However, it shares many design philosophies with Lua/Luau, particularly in variable declarations and a straightforward import/export system. In `via`, `tables` serve as the primary data structure, underpinning objects, namespaces, arrays, and dictionaries, which minimizes compatibility issues within the language.

### Key Features

- **Structured Types:** via introduces powerful features such as structs and static namespaces.
- **Control Structures:** The language supports constructs like switch statements for cleaner control flow.
- **Standard Library:** A robust standard library provides a wealth of functionality for developers.
- **Low-Level Memory Management:** Advanced memory management capabilities enable extra control over resource management.

### Variable Declarations

via includes three declaration keywords: `local`, `global`, and `property`. These can be modified with the `const` keyword to enforce immutability. 

- **Global Variables:** Immutable by default, global variables cannot be modified at runtime and are accessible throughout the document.
- **Local Variables:** These exist only for the duration of their scope, similar to Rust's lifetimes.
- **Properties:** Bound to objects, properties can be either mutable or immutable, depending on the presence of the `const` modifier. They are garbage collected only when the object is destroyed.

By building upon Lua's flexible yet primitive features, via provides a versatile and powerful programming environment that caters to a wide range of development needs.

### Datatypes

via has a small amount of *primitive* datatypes, which include:
- Number
- String
- Bool
- Table
- Nil

#### Number

Numbers in via are represented with a unified 64-bit floating point number type, which trades memory efficiency with compatibility and speed.

**Type name**: `Number` (returned by `type()`)
**Library**: `math`

#### String

Strings in via are basically an array of characters, wrapped with a lightweight safety mechanism.

**Type name**: `String`
**Library**: `string`

#### Bool

Booleans in via are pretty straight-forward, either `true` or `false`.

**Type name**: `Bool`
**Library**: `--`

#### Table

Tables in via are dynamic and complex data structures that are the backbone of arrays, dictionaries, objects, namespaces.

**Type name**: `Table`
**Library**: `table`

#### Nil

Nil is an empty value that represents `null` in other languages.

**Type name**: `Nil`
**Library**: `--`

## Basic examples

### Declarations

This declares a `mutable` variable with a value of `10`, and an automatically deduced type of `Number`.
```
local var = 10
```
We can make it immutable by using the `const` modifier.
```
local const var = 10
```
We can declare a global variable with the `global` keyword. Globals are immutable by default, and this property cannot be changed.
```
global var = 10
```
There's a third type of declarations in via, which are called properties. They are variables that are bound to objects, and declared using the `property` keyword.
```
## Inside object
property var = 10
```
They can be modified with the `const` keyword, making them immutable.

