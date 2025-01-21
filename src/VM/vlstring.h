/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "bytecode.h"
#include "state.h"
#include "api.h"
#include "types.h"
#include "libutils.h"

namespace via::lib
{

void string_format(RTState *);
void string_find(RTState *);
void string_sub(RTState *);
void string_format(RTState *);
void string_unpack(RTState *);
void string_char(RTState *);
void string_len(RTState *);
void string_format(RTState *);
void string_lower(RTState *);
void string_upper(RTState *);
void string_reverse(RTState *);
void string_byte(RTState *);
void string_match(RTState *);
void loadstringlib(RTState *);

} // namespace via::lib
