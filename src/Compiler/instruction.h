// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "opcode.h"

#ifndef VIA_OPERAND_COUNT
    #define VIA_OPERAND_COUNT 4
#endif

#define VIA_OPERAND_INVALID std::numeric_limits<U32>::max()

namespace via {

struct Chunk;

// Operand declarations
enum class OperandType {
    Nil,
    Number,
    Bool,
    String,
    Register,
};

struct Instruction {
    OpCode op = OpCode::NOP;
    U32 operand0 = 0;
    U32 operand1 = 0;
    U32 operand2 = 0;
    Chunk *chunk = nullptr;
    U64 pos = 0;

    Instruction() = default;
    Instruction(OpCode op, std::vector<U32> operands, Chunk *chunk, size_t pos)
        : op(op)
        , operand0(safe_call<U32>([&operands]() { return operands.at(0); }, VIA_OPERAND_INVALID))
        , operand1(safe_call<U32>([&operands]() { return operands.at(1); }, VIA_OPERAND_INVALID))
        , operand2(safe_call<U32>([&operands]() { return operands.at(2); }, VIA_OPERAND_INVALID))
        , chunk(chunk)
        , pos(pos)
    {
    }
};

std::string to_string(U32);
std::string to_string(ProgramData *, Instruction);

} // namespace via
