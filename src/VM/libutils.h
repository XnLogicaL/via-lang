/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "api.h"
#include "types.h"
#include "state.h"
#include "instruction.h"

#define LIB_ASSERT(cond, code, msg) \
    if (!(cond)) \
    { \
        ferror(msg); \
        setexitcode(V, code); \
        return; \
    }

#define WRAPVAL(val) TValue(new TCFunction(val, false))
#define ARG_MISMATCH(i, e, g) std::format("Expected {}, got {} for argument #{}\n", (e), (g), (i))
