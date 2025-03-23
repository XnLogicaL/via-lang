![Logo](https://i.imgur.com/Nyp3nLb.png)

![License](https://img.shields.io/github/license/XnLogicaL/via-lang)
![Latest commit](https://img.shields.io/github/last-commit/XnLogicaL/via-lang)
![Latest release](https://img.shields.io/github/v/release/XnLogicaL/via-lang)
![Language](https://img.shields.io/github/languages/top/XnLogicaL/via-lang)

# via

A performant, minimal, modern alternative to Lua.

# Warning

via is maintained by **one** person and may contain major security issues, bugs, and many other things you wouldn't want in production. Please reconsider using via in your project if you're not willing to take said risks.

# Motivation

While Lua is still widely used and loved by a lot of programmers all over the globe, it still has major flaws that make it bug-prone, unportable (in some cases) and too feature-lacking. via attempts to fix these issues by taking a familiar but completely different approach to Lua's goals. It retains Lua's simplicity by making most features optional to the programmer, while also providing very powerful features at the same time. It takes Lua's (specifically Luau's) type system and completely overhauls it with common metaprogramming practices, semi-static typing, and more. It also completely reimagines Lua's C/C++ API, making it more friendly to the end-user.

# Installation

## Standalone

Standalone binaries of via can be found in [Releases](https://github.com/XnLogicaL/via-lang/releases).

## Building from source

Building via from source only requires a few steps. If you're looking on how to embed via into your project using [CMake], {TBA}.

### Step 1: Cloning repository

First of all, you need to clone the repository into your preferred folder you want via.
#### Linux
```
path/to/repo$ git clone https://github.com/XnLogicaL/via-lang.git
```

### Step 2: Building

Once you've cloned the repo, you can now build via from source.
#### Linux
```
path/to/repo$ cd via
path/to/repo/via$ mkdir build
path/to/repo/via$ cd build
path/to/repo/via/build$ cmake ..
path/to/repo/via/build$ make -j$(nproc)
```