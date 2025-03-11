// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_BYTECODE_H
#define _VIA_BYTECODE_H

#include "instruction.h"

// ==========================================================================================
// bytecode.h
//
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
VIA_NAMESPACE_BEGIN

class BytecodeHolder final {
public:
    // Inserts a given bytecode pair to the vector.
    void add(const Bytecode&);
    // Removes the bytecode pair located in a given index.
    void remove(size_t);
    // Returns the current size_t or next index of the bytecode pair vector.
    size_t size() const noexcept;
    // Inserts a locally constructed instruction to a given index.
    void insert(
        size_t                        index    = 0,
        OpCode                        opcode   = OpCode::NOP,
        const std::array<Operand, 3>& operands = {},
        const std::string&            comment  = ""
    );
    // Emits an instruction at the end of the vector.
    void emit(
        OpCode                        opcode   = OpCode::NOP,
        const std::array<Operand, 3>& operands = {},
        const std::string&            comment  = ""
    );
    // Returns a reference to the bytecode vector.
    const std::vector<Bytecode>& get() const noexcept;

private:
    std::vector<Bytecode> instructions;
};

VIA_NAMESPACE_END

#endif
