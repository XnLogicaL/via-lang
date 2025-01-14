/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "instruction.h"
#include "codegen.h"

namespace via
{

// Logically bound chunk of instructions
// Generated by the compiler
struct Chunk
{
    // Pointer to the machine code of the chunk, function pointer
    JITFunc mcode = nullptr;
    // Pointer to the first instruction contained in the chunk
    Instruction *begin = nullptr;
    // Pointer to the last instruction contained in the chunk
    Instruction *end = nullptr;
    uint32_t pc;
};

} // namespace via