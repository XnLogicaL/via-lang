# Expressions

## Literal expressions

Literal expressions in via are defined as constant values that are known during compile time.

```
local x = 1 ## Literal
local msg = "Hello" ## Literal
local var = someOtherVar ## Not literal
```

### Literal types

via has multiple types of literals, which include:
- Integer literal
    - Example: `1, 50, -1, -6, 0x3C, 0b1001`
    - **Note**: Other numeric bases such as hexadecimal or binary are automatically converted into decimal literals during compilation.
- Floating point literal
    - Example: `1.0, 6.9, 31.4`
- String literal
    - Example: `"Hello", "Foo"`
- Nil literal
    - Example: `nil`
- Boolean literal
    - Example: `true, false`

## Unary expressions

Unary expressions are used to negate another expression. They are defined by the following grammar:

```
unary ::= -expr
```

The unary operator (`-`) can negate the number and boolean data types by default. Tables with the `__neg` metamethod can also be "negated".

## Binary expressions

Binary expressions are a combination of two expressions and an operator that form a valid binary operation. They are defined by the following grammar:

```
binary ::= expr (+|-|*|/|^|%) expr
```

These operators can only be used on number data types by default. However, if the left-hand-side expression of the binary expression is a table and has the appropriate metamethod associated with the operator, it will be called with both the expressions as arguments, in order.

Operator metamethod table:
| Operator | Metamethod |
| -------- | ---------- |
| `+`      | `__add`    |
| `-`      | `__sub`    |
| `*`      | `__mul`    |
| `/`      | `__div`    |
| `^`      | `__pow`    |
| `%`      | `__mod`    |

## Labmda expressions

Lambdas, also known as anonymous functions, are a functional programming "pattern" that make functions into values, a.k.a first class objects. They are defined by the following grammar:

```
lambda ::= func(strict|const parameter: type, ...) {
    statement...
}
```

