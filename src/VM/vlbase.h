// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

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
LIB_DECL_FUNCTION(base_weakPrimCast);
LIB_DECL_FUNCTION(base_strongPrimCast);
LIB_DECL_FUNCTION(open_baselib);

} // namespace via::lib
