/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "bytecode.h"
#include "state.h"
#include "api.h"
#include "types.h"
#include "libutils.h"

namespace via::lib {

void string_format(State *);
void string_find(State *);
void string_sub(State *);
void string_format(State *);
void string_unpack(State *);
void string_char(State *);
void string_len(State *);
void string_format(State *);
void string_lower(State *);
void string_upper(State *);
void string_reverse(State *);
void string_byte(State *);
void string_match(State *);
void loadstringlib(State *);

} // namespace via::lib
