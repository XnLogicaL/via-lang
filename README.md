<h1 align="center">
  <a href="https://github.com/XnLogicaL/via-lang">
    <img src="https://i.imgur.com/9WjzQ98.png" alt="via Language Logo"/>
  </a>
</h1>

<p align="center">
  <img src="https://img.shields.io/github/license/XnLogicaL/via-lang" alt="License">
  <img src="https://img.shields.io/github/languages/top/XnLogicaL/via-lang" alt="Top Language">
  <img src="https://img.shields.io/github/languages/count/XnLogicaL/via-lang" alt="Language Count">
  <img src="https://img.shields.io/badge/OS-linux%2C%20windows%2C%20macOS-0078D4" alt="OS">
  <img src="https://img.shields.io/badge/CPU-x86%2C%20x64%2C%20ARM%2C%20ARM64-FF8C00" alt="CPU">
  <img src="https://github.com/XnLogicaL/via-lang/actions/workflows/ci.yml/badge.svg" alt="CI">
</p>

<p align="center">
  This repository contains the <strong>via Programming Language</strong> source code, runtime, and tooling.
</p>

<p align="center">
  <a href="#introduction">Introduction</a> &nbsp;&bull;&nbsp;
  <a href="#features">Features</a> &nbsp;&bull;&nbsp;
  <a href="#installation">Installation</a> &nbsp;&bull;&nbsp;
  <a href="#credits">Credits</a>
</p>

# Introduction

**via** is a performant, multi-paradigm, deterministic scripting language designed for **performance-critical applications**.

```go
import std::io

fn ask_for_age_10_times() -> nil {
	for var i = 0, i < 10, i++ {
		var input = io::input("Enter your age: ")
		io::printn("You are {} years old!", input)
	}
}

ask_for_age_10_times()
```

>[!WARNING]
> This is an **experimental** project and not yet production-ready. Most features are under conception/development and may be incomplete/unstable. Implementations are subject to change as the project evolves.

# Features

- Non-intrusive static typing
- No garbage collector*
- Modern, clean and sane standard library and syntax
- Built-in types for strings, arrays, maps, tuples, optionals, unions, etc.
- Powerful metaprogramming
- Advanced compiler hints & intrinsics
- Multi-paradigm design, including object-oriented and functional programming
- High performance
- Platform independence*
- Rich C++ interface

# Installation

## Linux

### From source

Install the following binaries with your systems official package manager:
- `g++` (GCC 15 or above)
- `cmake`
- `ninja` (`ninja-build` in some repos)

Setup `vcpkg` using the [official vcpkg installation instructions](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell) and add these to your `~/.bashrc`/`~/.zshrc` (or similar):
```sh
export VCPKG_ROOT=/path/to/vcpkg
export PATH=$VCPKG_ROOT:$PATH
```
Or to define these variables temporarily, run these commands directly in the terminal instead.

Finally, restart your shell and optionally run these commands to test your `vcpkg` installation:
```sh
echo $VCPKG_ROOT
#> /path/to/vcpkg
which vcpkg
#> /path/to/vcpkg/vcpkg
```

Clone the official git repository and `cd` into it:
```sh
git clone https://github.com/XnLogicaL/via-lang.git /path/to/via
cd /path/to/via
```

Install dependency packages with the following command:
```sh
vcpkg install
```

Now generate build files using `cmake` and build & install the binaries:
```sh
cmake -B build -G Ninja -D CMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build # -j$(nproc) <- optional but recommended, as it significantly speeds up build times
# Optional:
cmake --install build
```

You can test your installation with this command:
```sh
which via
#> /bin/via
via
#> error: no input files
```

If you get a warning about the language core directory not being found or don't get output from one or more command, it probably means that the installation process failed. Try restarting your shell or creating an issue with appropriate information attached.

# Credits

- **@XnLogicaL** – Lead developer
- **@KasenDaniels** – Project name and banner design
- [MF TOOL - JAVA](https://github.com/ankitwasankar/mftool-java) – README layout inspiration
