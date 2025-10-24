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
  <a href="#design">Design</a> &nbsp;&bull;&nbsp;
  <a href="#installation">Installation</a> &nbsp;&bull;&nbsp;
  <a href="#credits">Credits</a>
</p>

>[!WARNING]
> via is an **experimental** project and not yet production-ready. Core features are under development and may be incomplete or untested. Implementations are subject to change as the project evolves.

# Introduction

**via** is a performant, multi-paradigm, deterministic scripting language designed for **performance-critical applications**.

# Installation

## Linux

### Pre-built

> [!WARNING]
> **via** currently does not ship pre-built binaries on Linux.

### From Source

> [!WARNING]
> The only tested compiler on Linux is **GCC**.
> **via** requires **GCC 15** or above to build.
> You can use `g++ --version` to check your compiler version.

#### Prerequisites

> [!INFO]
> Most people with established systems can safely skip this process.
> However, it is still highly recommended to follow these steps to ensure a proper build environment.

Install the following binaries with your systems official package manager:
- `g++`
- `cmake`
- `ninja` (`ninja-build` in some repos)

Setup `vcpkg` using the [official vcpkg installation instructions](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell) and add these to your `~/.bashrc` (or similar):
```sh
export VCPKG_ROOT=/path/to/vcpkg
export PATH=$VCPKG_ROOT:$PATH
```

Finally, restart your shell and optionally run these commands to test your `vcpkg` installation:
```sh
$ echo $VCPKG_ROOT
/path/to/vcpkg
$ which vcpkg
/path/to/vcpkg/vcpkg
```

---

First, clone the official git repository and `cd` into it:
```sh
$ git clone https://github.com/XnLogicaL/via-lang.git /path/to/via
$ cd /path/to/via
```

Now generate build files using `cmake` and build & install the binaries:
```sh
$ cmake -B build -G Ninja -D CMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
$ cmake --build build
$ cmake --install build
```

You can test your installation with this command:
```sh
$ which via
/bin/via
$ via
error: no input files
```

If you get a warning about the language core directory not being found or don't get output from one or more command, it probably means that the installation process failed. Try restarting your shell or creating an issue with appropriate information attached.

## Windows

> [!WARNING]
> Windows support is currently not tested.

### Pre-built

> [!WARNING]
> **via** currently does not ship pre-built binaries on Windows.

### From Source

TODO

## MacOS

> [!WARNING]
> **via** on MacOS is not officially supported, as I am too broke to affored a Mac.

### Pre-built

> [!WARNING]
> **via** currently does not ship pre-built binaries on MacOS.

### From Source

TODO

# Credits

- **@XnLogicaL** – Lead maintainer
- **@KasenDaniels** – Project name and banner design
- [mftool-java](https://github.com/ankitwasankar/mftool-java) – README layout inspiration
