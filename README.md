
![Logo](https://i.imgur.com/Nyp3nLb.png)

# via

A general purpose, performant, semi-dynamically typed scripting language. Moderately inspired by [Lua]()'s [Luau]() dialect.

See [Introduction](./docs/introduction.md) for more information.

# Warning

This is a **PASSION PROJECT**, it is maintained by **ONE** person and may contain vulnerabilities, design flaws, etc. Do not use in production if you think taking said risks is not worth it.

# Installation

## Pre-built binaries

{TBA}

## Building from source

To build via from source, you need `GNU Make`, `CMake`, and any modern C++ compiler. (default is `GNU g++`, using other compilers requires editing build files and may not work as intended, especially compilers such as `MSVC` that do not fully implement the C++23 standard)
First, we need to create a clean `build` directory. This can be done with the following commands:

### Linux
```bash
path/to/via$ mkdir build
path/to/via$ cd build
```

### Windows

{TBA}

---
The next step is to actually build the project, which we can do by first invoking `CMake` to generate build files, and follow it by invoking `Make` to compile into a binary.

### Linux

```bash
path/to/via/build$ cmake ..
path/to/via/build$ make -j$(nproc)
```

The `-j` flag in this context is used to dynamically speed up compilation using multithreading, as via is a farily large project and takes a long time to compile without multithreading. It is completely optional but heavily recommended to be included with the command. You can also specify the amount of CPU cores you'd like allocated by replacing `$(nproc)` with an integer.

### Windows

{TBA}

---

After you're done compiling via, you'll be left with a binary inside your build folder, which will function exactly like the pre-built binary distributions. Happy hacking!

A fully detailed installation guide can be found [here]().
