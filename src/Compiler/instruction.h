// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "opcode.h"

#ifndef VIA_OPERAND_COUNT
    #define VIA_OPERAND_COUNT 4
#endif

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
    size_t pos = 0;

    Instruction(OpCode, std::vector<U32>, Chunk *, size_t);
};

std::string to_string(ProgramData *, Instruction);

} // namespace via
