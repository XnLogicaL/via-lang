// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_INSTRUCTION_H
#define _VIA_INSTRUCTION_H

#include "common.h"
#include "opcode.h"

#define VIA_OPERAND_INVALID std::numeric_limits<Operand>::max()

VIA_NAMESPACE_BEGIN

using Operand  = u16;
using OperandS = i16;

struct Chunk;
struct InstructionData {
    Chunk*      chunk   = nullptr;
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

std::string to_string(const Bytecode&, bool);

VIA_NAMESPACE_END

#endif
