# Contributing to via

Thank you for your interest in contributing to the **via programming language**!  
We welcome issues, pull requests, ideas, and feedback.

---

## Getting Started

1. **Fork the repository** and clone your fork:
   ```bash
   git clone https://github.com/XnLogicaL/via-lang.git
   cd via-lang
   ```

2. **Build the project**:
   ```bash
   cmake -B build -D CMAKE_BUILD_TYPE=Debug
   cmake --build build
   ```

---

## Where to Contribute

- 🧠 **Bytecode Interpreter**: Instruction set, execution model, memory system
- 📦 **Compiler/Parser**: Frontend logic, AST, bytecode generation
- 🧹 **Diagnostics & Error Handling**
- 📖 **Docs**: Clarifications, grammar rules, developer notes
- 🧪 **Tests**: Add minimal reproducible tests for language features
- 🧩 **Standard Library**

---

## Code Style

- Use **modern C++** (C++20 and later).
- Avoid raw memory management at all costs.
- Prefer in-house abstractions where applicable.
- Follow existing conventions:
  - Follow the [Format Guidelines](./.clang-format).
  - Use `snake_case` for variables and functions.
  - Use `PascalCase` for classes and types.
  - Use `LOUD_SNAKE_CASE` for macros.
  - Use spaces instead of tabs when indenting.
  - Be consistent with changes.
  - Keep headers light and self-contained.

---

## Pull Request Guidelines

- Open a draft PR early if unsure — discussion is welcome.
- Keep PRs focused and minimal. One feature/fix per PR is ideal.
- Include comments for complex logic.
- If you add a new feature, add a minimal test case if possible.

---

## Reporting Issues

- Use the [Issues](https://github.com/XnLogicaL/via-lang/issues) tab.
- Include:
  - What you expected vs. what happened
  - Minimal reproducible example
  - Version/commit hash if possible

---

## Licensing

All contributions must be compatible with the [GNU GPL v3](./LICENSE).

---

Thanks again for helping make via better!
