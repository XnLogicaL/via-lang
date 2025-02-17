/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "api.h"
#include "libutils.h"
#include "types.h"
#include "state.h"

namespace via::lib {

LIB_DECL_FUNCTION(base_print);
LIB_DECL_FUNCTION(base_println);
LIB_DECL_FUNCTION(base_error);
LIB_DECL_FUNCTION(base_assert);
LIB_DECL_FUNCTION(base_weak_prim_cast);
LIB_DECL_FUNCTION(base_strong_prim_cast);
LIB_DECL_FUNCTION(open_baselib);

} // namespace via::lib
