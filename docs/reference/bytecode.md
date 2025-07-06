# Bytecode

via does not use protos to contain bytecode. Instead, it uses a linear instruction array.

## Instruction

An instruction is a fixed sized 8-byte structure that contains a 2-byte [Opcode](https://en.wikipedia.org/wiki/Opcode), and three 2-byte operands. This structure is defined as the following:

```cpp
enum Opcode : uint16_t { ... }

struct Instruction {
  Opcode op;
  uint16_t a, b, c;
}
```

## Header

A "Header" refers to a structure that contains full runtime context for a via program. It contains crucial runtime data like program constants, flags, bytecode, etc. This structure is defined as the following:

```cpp
struct Header {
  uint32_t magic = 0xDEADCAFE;
  uint64_t flags;
  uint32_t const_count;
  Constant consts[const_count];
  uint32_t bytecode_count;
  Instruction bytecode[bytecode_count];
}
```

Constants are a serialized form of interpreter values, and are defined as the following:
```cpp
struct Constant {
  uint8_t tag;
  uint8_t data[];
}
```

Constants must define data with the size denoted along with the following tags:
- VLK_NIL `0x00` 0-byte(s)
- VLK_INT `0x01` 4-byte(s)
- VLK_FLOAT `0x02` 4-byte(s)
- VLK_BOOLEAN `0x03` 1-byte(s)

The data array is then to be reinterpreted as the corresponding data type as an interpreter value.

The execution entry point for the interpreter is fixed and is defined as `Header::bytecode[0]`. It is assumed that the instruction array contains at least one instruction and is terminated by a `RET` instruction. Failure to comply with said requirements will invoke [undefined behavior](https://en.wikipedia.org/wiki/Undefined_behavior) in the interpreter.


