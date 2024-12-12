
![Logo](https://i.imgur.com/Nyp3nLb.png)

[![GitHub Releases](https://img.shields.io/github/release/xnlogical/json.svg)](https://github.com/xnlgoical/via-lang/releases)

# via

A general purpose, performant, semi-dynamically typed scripting language.
Heavily inspired by Lua's "Luau" dialect.

See [Introduction](./docs/introduction.md) for more information.

# Warning

This project is currently far from completion and has been rewritten countless times.
Expect unintended behavior, bugs, etc.
Do not use in production if you think taking said risks is not worth it.

# Build

To build via, you need `Make`, `python3` and any modern C++ compiler (default is `g++`, using others requires editing the `Makefile` and may not work as intended).

## Step 1: Install dependencies

{TBA}

## Step 2: Run build

Now that you have installed all the dependencies, you can now build via from source.
First things first, select one of these "specifications":
- via (full via language)
- viac (via compiler, compiles via code into bytecode and outputs)
- viavm (via virtual machine, interprets bytecode from a file)

To run the build:
### Linux
```$ python3 scripts/build.py <your_build_spec>```

### Windows
```python scripts/build.py <your_build_spec>```

## Step 3: Run program

Your build will be located in the $root/build directory, you can now run the extensionless executable named after your selected spec.