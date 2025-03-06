// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "opcode.h"

#ifndef VIA_OPERAND_COUNT
    #define VIA_OPERAND_COUNT 4
#endif

#define VIA_OPERAND_INVALID std::numeric_limits<Operand>::max()

namespace via {

using Operand  = U16;
using OperandS = I16;

struct Chunk;
struct InstructionData {
    Chunk      *chunk   = nullptr;
    std::string comment = "";
};

struct alignas(8) Instruction {
    OpCode  op       = OpCode::NOP;
    Operand operand0 = 0;
    Operand operand1 = 0;
    Operand operand2 = 0;
};

struct alignas(64) Bytecode {
    Instruction     instruction;
    InstructionData meta_data;
};

std::string to_string(const Bytecode &);

} // namespace via
