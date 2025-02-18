/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "vmapi.h"

#define LIB_ASSERT(cond, message) \
    if (!(cond)) { \
        impl::__set_error_state(V, message); \
        return; \
    }

#define LIB_DECL_FUNCTION(id) void id(State *V)
#define LIB_DECL_PARAMETER(id, idx) const TValue &id = impl::__get_argument(V, idx);
#define LIB_RETURN(retc) impl::__native_return(V, retc);

#define LIB_WRAP_CFPTR(ptr) (TValue(new TCFunction(ptr)))
#define LIB_WRAP_PRIM(val) (TValue(val))

#define LIB_ERR_ARG_TYPE_MISMATCH(type0, type1, parameter) \
    LIB_ASSERT(false, std::format("Expected {}, got {} (parameter #{})", type0, type1, parameter));
