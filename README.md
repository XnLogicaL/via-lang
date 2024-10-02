
![Logo](https://i.imgur.com/Nyp3nLb.png)

[![GitHub Releases](https://img.shields.io/github/release/xnlogical/json.svg)](https://github.com/xnlgoical/via-lang/releases)

# via

A fast, dynamically typed, jit compiled scripting language.

via inherits many features from a variety of different languages including but not limited to:
`Luau`, `GoLang`, `TypeScript`, `C`, etc.

via uses a JIT (Just-in-time) compiler to compile your code into bytecode and interprets it in the via virtual machine.
via has a lot more runtime errors compared to compilation errors due to the interpreted and highly dynamic nature of the language.

## Disclaimers

#### THIS IS A PASSION PROJECT!
via is a passion project which means that it might have security flaws, missing features and a lot more issues
given the small amount of people who develop and maintain via.
We don't encourage you to use via in a production environment. 
If done so, we are not responsible for any damages caused by the unstable nature of the language.

## Features

- Classes
- Lambdas (has the same type as functions)
- Verbose debugging functions: `error` and `warn`
- Simple curly syntax
- Simple type semantics
- Scoped declarations (possible via the  "local" keyword)
- Global declarations (possible via the "global" keyword)
- Constant variables (using the "!" "operator")
- All default types are primitive (they will have corresponding wrappers in the std library, this allows for better low level control)
- Modules & libraries
    - 1: Make a via module file with the `.viam` extension (files that don't have this ext cannot be imported)
    - 2: Declare a module with `module <moudle_name>`
    - 3: Export the module with `export <module_name>`
    - 4: Import your module with `using "path/to/module.viam"`

**Planned**:
- Package manager
- Extensive std library
- Embedding API
- Rust version

## Contributing

via isn't currently open for public contribution.

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
local myInt: char = 10 ## You can do this, but it's not good practice!
```

#### If statements
```
if (true) {
    print("This will print")
} else {
    print("This won't print")
}

## Alternatively;

if (true):
    print("This will print")
else:
    print("This won't print")

## This syntax can only be used for 1-line statements
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
fn! myFunction() {
    print("Hello from myFunction!")
}

myFunction() ## "Hello from function!"
```

#### Scopes
```
do {
    ## Do stuff inside Scopes
    fn! funcInsideScope() {
        print("Hello from scope")
    }

    funcInsideScope() ## "Hello from scope"
}
```

#### Tables
*Tables are primtive data structures that can represent arrays, dictionaries, etc.*
```
local myDict = {
    ["ten"] = 10,
    ["nine"] = 9,
    eight = 8,
    seven = 7
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

    fn! new(value: int) {
        self:value = value
    }

    fn! destroy() {
        print("Destroying class!")
    }

    fn! print_value() {
        ## `self` is automatically declared in every function inside a class
        print(self:value)
    }
}

local classInstance = new MyClass(0) ## Calls the constructor (class:new) automatically
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
using "path/to/module.viam" ## as MyModule <- optional, will automatically be assigned an identifier

print(MyModule:var) ## "This will print!"
```

#### Pointers
```
local pInt: ptr<int> = &10
local Int = *pInt

local pNull = 00000000 ## nullptr
local Null = *pNull ## "error: attempt to dereference nullptr"
```