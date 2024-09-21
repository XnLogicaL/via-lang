
![Logo](https://i.imgur.com/Nyp3nLb.png)

[![GitHub Releases](https://img.shields.io/github/release/xnlogical/json.svg)](https://github.com/xnlgoical/via-lang/releases)

# via

A fast, statically and dynamically typed programming language.

via has inherited many features from a variety of different languages including but not limited to: `Luau`, `GoLang`, `TypeScript`, `C`, etc.

(via has a compiled, interpreted and jit compiled versions in the works/available)

#### Big thanks to [@orosmatthew](https://github.com/orosmatthew) for the parser structure

## Disclaimers

#### THIS IS A PASSION PROJECT!
via is a passion project which means that it might have security flaws, missing features and a lot more issues.
We heavily discourage you to use via in a production environment.

## Features

- Classes (no inheritance/polymorphism)
- Verbose debugging functions: `error` and `warn`
- Simple curly syntax
- Simple type semantics
- Scoped declarations (possible via the  "local" keyword)
- Global declarations (possible via the "global" keyword)
- All default types are primitive (they will have corresponding wrappers in the std library, this allows for better low level control)
- Modules & libraries
    - 1: Make a via module file with the `.viam` extension (files that don't have this ext cannot be imported)
    - 2: Declare a module with `module <moudle_name>`
    - 3: Export the module with `export <module_name>`
    - 4: Import your module with `using "path/to/module.viam"`



**Planned**:
- Package manager
- Extensive std library
- Interpreted version (W.I.P)
- JIT (compiled & interpreted) version
- Embedding support
- Rust version
- LLVM version

## Usage/Examples

#### Comments
```
## This is a comment that will be ignored by the compiler
```

#### Variables
```
local var = 10 ## Automatically assigned type "int"
```

#### Constants
```
local! var = 10 ## Cannot be reassigned
```

#### Type annotations
```
local myInt: int = 10
local myInt: char = 10 ## This will error during compile time
```

#### If statements
```
if (true) {
    print("This will print")
} else {
    print("This won't print")
}
```

#### Loops
```
for (i, v in {...}) {
    print(i) ## 0, 1, 2, 3, 4, etc.

    if (i == 1) {
        break
    } else {
        continue
    }
}

while (true) {
    break
}
```

#### Functions
```
local! myFunction = function() {
    print("Hello from function!")
}

myFunction() ## "Hello from function!"
```

#### Scopes
```
do {
    ## Do stuff inside Scopes
    local! funcInsideScope = function() {
        print("Hello from scope")
    }

    funcInsideScope() ## "Hello from scope"
}
```

#### Tables
*Tables are primtive data structures that can represent arrays, dictionaries, structs, etc.*
```
local myDict = {
    ["ten"] = 10,
    ["nine"] = 9
}

local myArray = {
    1,
    2,
    [2] = 3,
    [3] = 4,
}
```

#### Error handling
```
try {
    ## ...
} catch(err: string) {
    print("failed because: " .. err)
}
```

#### Classes
```
class MyClass {
    local! value: int

    new(value: int) {
        self:value = value
    }

    destroy() {
        print("Destroying class!")
    }

    print_value() {
        print(self:value)
    }
}

local classInstance = new MyClass(0)
classInstance:print_value() ## "0"

destroy classInstance ## "Destroying class!"
```

#### Modules
```
## module.viam
module MyModule ## Start module scope

print("This will print!")

local var = "This will be exported as module:var"

export MyModule ## End module scope
```

```
using "path/to/module.viam" ## as MyModule <- optional

print(MyModule:var) ## "This will print!"
```

#### Pointers
```
local pInt: ptr<int> = &10
local Int = *pInt

local pNull = 00000000 ## nullptr
local Null = *pNull ## "error:  attempt to dereference nullptr"
```