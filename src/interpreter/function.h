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
    bool is_error_handler = false;
    bool is_vararg        = false;

    Instruction* ret_addr = nullptr;
    Instruction* bytecode = nullptr;
    TFunction*   caller   = nullptr;
    UpValue*     upvs     = new UpValue[VIA_UPV_COUNT];

    uint32_t bytecode_len = 0;
    uint32_t upv_count    = VIA_UPV_COUNT;

    VIA_DEFAULT_CONSTRUCTOR(TFunction);
    VIA_CUSTOM_DESTRUCTOR(TFunction);

    TFunction(bool is_error_handler, bool is_vararg, Instruction* return_address, TFunction* caller)
        : is_error_handler(is_error_handler),
          is_vararg(is_vararg),
          ret_addr(return_address),
          caller(caller) {}

    TFunction(const TFunction& other);
};

struct TCFunction {
    void (*data)(State*)  = nullptr;
    bool is_error_handler = false;
};

VIA_NAMESPACE_END

#endif
