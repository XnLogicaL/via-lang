/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#define LIB_ASSERT(cond, code, msg) \
    if (!(cond)) \
    { \
        ferror(msg); \
        setexitcode(V, code); \
        return; \
    }

#define LIB_WRAP_CFPTR(ptr) (TValue(new TCFunction(ptr)))
#define LIB_WRAP_PRIM(val) (TValue(val))

#define LIB_ERR_ARG_TYPE_MISMATCH(type0, type1, parameter) \
    LIB_ASSERT(false, VMEC::unexpected_argument, std::format("Expected {}, got {} (parameter #{})", type0, type1, parameter));
