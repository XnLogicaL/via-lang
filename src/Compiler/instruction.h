// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "opcode.h"

#ifndef VIA_OPERAND_COUNT
    #define VIA_OPERAND_COUNT 4
#endif

#define VIA_OPERAND U16
#define VIA_OPERAND_S I16
#define VIA_OPERAND_INVALID std::numeric_limits<VIA_OPERAND>::max()

namespace via {

struct Chunk;
struct InstructionData {
    Chunk *chunk = nullptr;
    std::string comment = "";
};

struct alignas(8) Instruction {
    OpCode op = OpCode::NOP;
    U16 operand0 = 0;
    U16 operand1 = 0;
    U16 operand2 = 0;
};

struct alignas(64) Bytecode {
    Instruction instruction;
    InstructionData meta_data;
};

std::string to_string(VIA_OPERAND_S);
std::string to_string(const Bytecode &);

} // namespace via
