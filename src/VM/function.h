// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_FUNCTION_H
#define _VIA_FUNCTION_H

#include "common.h"
#include "state.h"
#include "object.h"

#define VIA_UPV_COUNT 64

VIA_NAMESPACE_BEGIN

struct UpValue {
    bool    is_open    = true;
    TValue* value      = nullptr;
    TValue  heap_value = TValue();
};

struct TFunction {
    u32         line = 0;
    const char* id   = "<anonymous-function>";

    bool is_error_handler = false;
    bool is_vararg        = false;

    Instruction* ret_addr = nullptr;
    Instruction* bytecode = nullptr;
    TFunction*   caller   = nullptr;
    UpValue*     upvs     = new UpValue[VIA_UPV_COUNT];

    u32 bytecode_len = 0;
    u32 upv_count    = VIA_UPV_COUNT;
};

struct TCFunction {
    void (*data)(State*)  = nullptr;
    bool is_error_handler = false;
};

VIA_NAMESPACE_END

#endif
