# via

**Disclaimer**
This project is heavilty being worked on and currently is not able to execute code.
 
A compiled, fast, statically typed general purpose language.
via encompasses different features from a variety of different languages such as Lua, Go, TypeScript, etc.
This project is mainly for learning & improving in C++ and making for a valuable portfolio asset.
Feel free to code stuff with it.

# Credits
- Thanks to @OrosMatthew for parser code inspiration, make sure to check his YouTube series out!

# Examples
Planned/completed features for via.

Declare variables with `local` or `global`
```
local var: int = 10
```

Add an exclamation mark after the `local` keyword to make it a constant
```
local! var: int = 10
```

Add comments with `##`
```
## this is a comment
```

All functions are lambda functions in via
```
local! add: function = function(a: int, b: int) {
  return a + b
}
```
(Note: the `function` type is a placeholder for semantics) 

Create classes with the `class` keyword
And constructors with `new`
And destructors with `destroy`
```
class MyClass {
  myProperty: int = 10

  new() {
    print("This class is quite cool")
  }
  destroy() {
    print("Not so cool anymore")
  }
}
```

Access methods/properties of a class using colons
```
local myClass: auto = new MyClass() ## "This class is quite cool!"

print(myClass:myProperty) ## "10"

destroy myClass ## "Not so cool anymore"
```

Don't worry about inheritance as via doesn't have dumb features like that. A class should have it's own methods and it's own methods only.
