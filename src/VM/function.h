// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "state.h"

#define VIA_UPV_COUNT 64

namespace via {

struct TValue;
using UpValue = TValue *;

struct TFunction {
    U32         line = 0;
    const char *id   = "<anonymous-function>";

    bool is_error_handler = false;
    bool is_vararg        = false;

    Instruction *ret_addr = nullptr;
    Instruction *bytecode = nullptr;
    TFunction   *caller   = nullptr;
    UpValue     *upvs     = new UpValue[VIA_UPV_COUNT];

    U32 bytecode_len = 0;
    U32 upv_count    = VIA_UPV_COUNT;
};

struct TCFunction {
    void (*data)(State *) = nullptr;
    bool is_error_handler = false;
};

} // namespace via
