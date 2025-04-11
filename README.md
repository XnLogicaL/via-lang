![Logo](https://i.imgur.com/Nyp3nLb.png)

![License](https://img.shields.io/github/license/XnLogicaL/via-lang)
![Latest commit](https://img.shields.io/github/last-commit/XnLogicaL/via-lang)
![Latest release](https://img.shields.io/github/v/release/XnLogicaL/via-lang)

# via

*"An evolution, not a revolution. -C3"*

A performant, minimal, modern alternative to **Lua**.

**via** is a language that attempts to fix the issues of modern interpreted languages, rather than adding more bloat and abstractions over them.

It takes good features from all respected and established languages and carefully sows them together to hopefully make for a good scripting language.

## Why not Lua?

After using Lua for many projects, you come to realize that the language has many flaws that drive you away from it in a lot of cases.

One of these flaws is the **identity crisis** and **ambigiouty** when it comes to **what is a core feature** and what is a **standard library feature**.

via solves this problem by only abstracting what is necessary. All of the languages features are within the scope of the capabilities of the syntax and semantics of the language. It only abstracts things like IO and OS level interactions to the C++ backend standard library, which is otherwise impossible to implement due to how interpreted languages work.

**However**, this does not imply that Lua is a bad language in any way. In fact I would consider it the father of many modern interpreted languages. It is a brilliant piece of engineering and the intention of this project is not to replace it, but to build upon its **best features** and not fall into the pitfalls of its **mistakes**.

---

## Design

#### Syntax

via has a semi-verbose syntax that is designed to get the behavior of code across with just a look.

- Semicolons can be used but not required. This is because modern lexical analysis really deprecates the need for semicolons and explicit statement termination in general, given the language has a robust and stable lexical structure.

- All syntax that could be ambigious to the user is carefully designed to dissolve that ambigiouty. It replaces symbols with keywords in a lot of places, such as conditionals, where all operators like "*&&*" (and), "*||*" (or), "*!*" (not) are replaced with their more verbose keyword counterparts respectively: "*and*", "*or*", "*not*".

- via uses curly-braces ("*{*" and "*}*") to define scope bounds, which makes it familiar to a significant portion of programmers, while not losing the appeal of simplicity.

#### Static typing

via is a statically typed language. However, the type system is implemented in a way that, it does not make the language any less ergonomic and easy-to-use.

- It has been proven many times that programmers tend to favor languages that offer type safety, rather than loosely typed languages. A great example to this claim is the [TypeScript]() project, which was made to fix the pretty much non-existant type system of JavaScript, and today, it is one of the most desired programming languages out there.

- Static typing also has another massive benefit, which significant runtime performance improvements over dynamic typing, at the very cheap cost of more compile-time checks. In a lot of cases, via outperforms Lua by 1.5-2x, simply because the interpreter does not have to check types and trusts the compiler.

#### Simplicity

Much like its predecessor, via treats simplicity as a core design principle.

- via does **not** and will **never** have things like **async**, **inheritance**, and among other things that introduce a unfavorable amount of complexity/ambigiouty to the language that either conceal too much behavior, or outright exist to conceal bad language design.

#### Performance

via has an **extremely fast** runtime, often outperforming reputably fast interpreted languages like Lua, without stepping into **JIT** territory.

- It is bytecode interpreted, which means that it will always have fundemental overhead that puts it behind compiled languages.

- It uses a C++ backend that provides an interpreter along with many standard library functions that require ABI calls.

- It has a powerful and often aggressively-optimizing compiler that has the capability to compiler user-defined functions into native machine code and still treat them as functions during runtime, achieving close to native performance.

#### Embedability

via is designed to be embedable into most environments that require an efficient scripting language.

- It is a **minimally-sized** project, it currently hosts about ~15k lines of code, which means that you can place it pretty much whereever you would like. 

- It is **memory-efficient**, using priniciples like RAII under the hood to ensure maximum memory efficiency.

- It is hosted on top of a C++ backend, giving you **almost** full control of the sandbox environment of the language.

---

## Installation

#### Prebuilt

To install pre-built binaries, simply go to the [GitHub Releases]() page and pick the binary that suits your OS and CPU architecture.

#### From source

If pre-built installation is not available to you for whatever reason, or you just want to mess around with the source code, you're more than welcome.

**Linux** is recommended for this process.

##### Preperation: Installing dependencies
via uses [CMake]() to generate build files. Compilers and other tools are preference-based, here are supported compilers:
- MSVC
- Clang
- GCC

You can verify that you have `CMake` installed by running this:
```bash
$ cmake --version
```

---

##### Step 1: Cloning the repository

First step is to clone the repository into your local machine.
```bash
$ git clone --recursive https://github.com/XnLogicaL/via-lang.git
```

##### Step 2: Generating build files

Now that you have the repository cloned, you will need to make a `build` folder.
```bash
$ mkdir build && cd build
```

After you have done that, you can go ahead and actually generate build files.
```bash
$ cmake .. -DDEBUG_MODE=OFF
```
The `-DDEBUG_MODE=OFF` flag disables debug mode, which is used for testing and disables optimizations.

##### Step 3: Building executable

Finally, you can build the executable by running your build system. If you are on Linux, this will be `Make`, on Windows it will be `msbuild`. For the sake of the guide, we will use Linux for this.
```bash
$ make -j$(nproc)
```
After make finishes building, you should be left with a statically linked executable that should work anywhere on any device as long as the OS and CPU architecture are consistent with where it was compiled.

You can verify that the executable works with this command:
```bash
$ ./via --version
via XX.XX.XX
```

## Credits

**XnLogicaL** - Lead maintainer
**Prismic (Kasen L. Daniels)** - Name and banner
