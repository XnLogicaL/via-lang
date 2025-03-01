// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "instruction.h"
#include "codegen.h"

namespace via {

// Logically bound chunk of instructions
// Generated by the compiler
struct Chunk {
    // Pointer to the machine code of the chunk, function pointer
    void *mcode = nullptr;
    // Pointer to the first instruction contained in the chunk
    Instruction *begin = nullptr;
    // Pointer to the last instruction contained in the chunk
    Instruction *end = nullptr;
    U32 pc;
};

} // namespace via