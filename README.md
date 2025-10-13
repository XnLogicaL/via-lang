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

### Debian-based systems

> **Warning:**
> `via` requires **GCC 15** to build. As of `13/10/2025`, GCC 15 is not yet in stable `apt` repositories, so you may need to build it manually.

Install the prerequisites using `apt`:

```bash
sudo apt install cmake g++ ninja-build git
```

Install `vcpkg` using the [official Microsoft tutorial](https://vcpkg.io/en/getting-started).
After installation, configure your environment variables:
```bash
# For user-local installation:
echo "export VCPKG_ROOT=/path/to/vcpkg" >> ~/.bashrc
echo "export PATH=\$VCPKG_ROOT:\$PATH" >> ~/.bashrc
source ~/.bashrc
```

Clone the repository:
```bash
git clone https://github.com/XnLogicaL/via-lang.git /path/to/via
cd /path/to/via
```

Run the install script that corresponds to your installation goals; `install-deb.sh` for system-wide installation, and `install-user-deb.sh` for local installation:
```bash
# User-local installation (recommended):
scripts/linux/install-user-deb.sh
# System-wide installation:
sudo scripts/linux/install-deb.sh
```

These scripts will automatically:
- Build the interpreter and core libraries
- Copy the executable to your binary directory
- Set up the core libraries in the appropriate locatio

# Credits

- **@XnLogicaL** – Lead maintainer
- **@KasenDaniels** – Project name and banner design
- [mftool-java](https://github.com/ankitwasankar/mftool-java) – README layout inspiration
