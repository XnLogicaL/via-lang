// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "function.h"

#define DELETE_IF(target)                                                                          \
    if (target) {                                                                                  \
        delete target;                                                                             \
        target = nullptr;                                                                          \
    }

#define DELETE_ARR_IF(target)                                                                      \
    if (target) {                                                                                  \
        delete[] target;                                                                           \
        target = nullptr;                                                                          \
    }

VIA_NAMESPACE_BEGIN

TFunction::~TFunction() {
    DELETE_ARR_IF(upvs);
    DELETE_ARR_IF(bytecode);
}

TFunction::TFunction(const TFunction& other)
    : is_error_handler(other.is_error_handler),
      is_vararg(other.is_vararg),
      ret_addr(other.ret_addr),
      caller(other.caller),
      bytecode_len(other.bytecode_len),
      upv_count(other.upv_count) {
    bytecode = new Instruction[bytecode_len];
    upvs     = new UpValue[upv_count];

    std::memcpy(bytecode, other.bytecode, bytecode_len);
    std::memcpy(upvs, other.upvs, upv_count);
}

VIA_NAMESPACE_END
