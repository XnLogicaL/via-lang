/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "api.h"
#include "libutils.h"
#include "types.h"
#include "state.h"

namespace via::lib
{

LIB_DECL_FUNCTION(base_print);
LIB_DECL_FUNCTION(base_println);
LIB_DECL_FUNCTION(base_error);
LIB_DECL_FUNCTION(base_exit);
LIB_DECL_FUNCTION(base_type);
LIB_DECL_FUNCTION(base_typeof);
LIB_DECL_FUNCTION(base_assert);
LIB_DECL_FUNCTION(loadbaselib);

} // namespace via::lib
