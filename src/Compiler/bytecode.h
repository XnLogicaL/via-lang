// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "instruction.h"

// ================================================================ |
// File bytecode.h: BytecodeHolder declarations.                    |
// ================================================================ |
// This file declares the BytecodeHolder class.
//
// The BytecodeHolder class serves as an abstraction over a container
//  (such as std::vector), providing methods like `add`,
//  `remove` and `emit`. The main reason behind it's
//  existence is to provide a more "fit-for-duty" interface for
//  interfacing with a bytecode array, specifically in the context of compilation.
//
// The `emit` method (the most important method) is used to emit an
//  instruction *without* actually constructing it on the spot,
//  but rather constructing it inside the method, and is widely used
//  inside the current compiler implementation.
//
// `emit` also provides the caller with an option to add a comment, which
//  is in turn saved to ProgramData::bytecode_info.
// ================================================================ |
namespace via {

class BytecodeHolder {
public:
    BytecodeHolder(ProgramData *program)
        : program(program)
    {
    }

    void add(const Bytecode &);
    void remove(U64 index);
    void insert(
        U64 index = 0,
        OpCode opcode = OpCode::NOP,
        const std::array<VIA_OPERAND, 3> &operands = {},
        const std::string &comment = ""
    );

    void emit(
        OpCode opcode = OpCode::NOP,
        const std::array<VIA_OPERAND, 3> &operands = {},
        const std::string &comment = ""
    );

    std::vector<Bytecode> &get();

private:
    ProgramData *program;
    std::vector<Bytecode> instructions;
};

} // namespace via