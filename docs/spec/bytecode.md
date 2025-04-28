# Spec XX: Bytecode

The term "bytecode" refers to a structure that contains both an instruction and instruction metadata. This structure is defined as follows:
```cpp
struct Bytecode {
  Instruction insn;
  InstructionData meta;
}
```


## Subspec 00: Instruction Format

Instructions consist of an **opcode**, and **three operands**. Each component — including the opcode (stored as an enum) — is represented as a 2-byte unsigned integer. The full structure is exactly 8-bytes in size, making it efficient for memory and cache alignment.

An instruction is defined as follows:
```cpp
struct Instruction {
  Opcode op;
  uint16_t a, b, c;
}
```

This structure is **extremely** efficient with space and cache-lines as it is exactly 8-bytes in size, therefore can be aligned by 8-bytes.

Operands can be "invalid" by holding the value `0xFFFF` or `UINT16_MAX`. This is used in instruction to string conversions, to reduce clutter in instruction disassemblies.

> [!NOTE]
> The value `0xFFFF` (in the context of an operand holding it) carries no semantic meaning during execution and is treated exactly the same as any other operand by the interpreter.

## Subspec 01: Opcodes

A full list of opcodes can be found in [opcode.h]().

Opcodes are ordered in the interpreter from most likely to be executed to least likely to be executed. This micro-optimization especially helps in the case of compilation using the MSVC compiler or similar compilers of which do not support dynamic dispatch tables.

All applicable opcodes must provide operands in this exact order:
- Registers (destination `dst` first)
- Identifiers (in the format of integers)
- Literals (usually in the format of a high operand and a low operand)

### Opcode suffixes
- `I` (32-bit integer literal, encoded into two operands)
- `F` (32-bit IEEE-754 floating pointer literal, encoded into two operands)
- `BF` (8-bit boolean literal `false`, encoded into one operand)
- `BT` (8-bit boolean literal `true`, encoded into one operand)
- `K` (16-bit constant id)

Opcodes `ADDI, ADDF, SUBI, SUBF, MULI, MULF, DIVI, DIVF, POWI, POWF, MODI, MODF, LOADI, LOADF, PUSHI, PUSHF` encode different types of literals (mainly IEEE-754 32-bit floating point numbers, 32-bit integers) into two operands, resulting in a larger 32-bit bitspace for the literal being loaded.

> [!IMPORTANT]
> Opcodes that encode 32-bit literals reinterpret operands using big-endian encoding:
>
> ```cpp
> uint32_t((low_operand << 16) | high_operand)
> ```
>
> The high operand **must** appear first in the instruction (i.e., before the low operand).

---

Some opcodes hold special "sentinel/pragma" roles in the interpreter. They typically have no runtime behavior, and almost all of them are defined as `NOP`s.

These instructions are as follows:
- `LABEL <x>` - This instruction is used in the "preprocessing" stage of the interpreter where the main function is linearly scanned to map labels.

## Subspec 02: 

