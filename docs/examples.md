## Variables

Local variables can be declared using the `var` keyword: 
```swift
var x = 10
```

Constant variables can be declared by adding the `const` keyword after the declaration keyword:
```js
var const x = 10
```

This can also be shortened by simply removing the `var` keyword:
```js
const x = 10
```

Type annotations can be added using a `:` after the variable identifier:
```js
var x: int = 10 ## Ok
var x: string = 10 ## Error!
```

## Globals

Global variables can be decalred using the `global` keyword:
```py
global x = 10
```

Global variables are immutable and constant, therefore they cannot be redeclared or reassigned. `const` modifiers are also ineffective when used in combination with `global`, and they will raise a compiler warning.