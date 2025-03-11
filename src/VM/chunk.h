// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_CHUNK_H
#define _VIA_CHUNK_H

#include "common.h"
#include "instruction.h"
#include "codegen.h"

VIA_NAMESPACE_BEGIN

struct Chunk {
    void*        mcode = nullptr;
    Instruction* begin = nullptr;
    Instruction* end   = nullptr;
    u32          pc;
};

VIA_NAMESPACE_END

#endif
