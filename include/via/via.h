/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#if 1
    #include "config.h"
    #include "policy.h"
#endif

#include <ast/ast.h>
#include <diagnostics.h>
#include <lexer/lexer.h>
#include <lexer/location.h>
#include <lexer/token.h>
#include <memory.h>
#include <module/defs.h>
#include <module/manager.h>
#include <module/module.h>
#include <module/symbol.h>
#include <parser/parser.h>
#include <sema/const_value.h>
#include <sema/register.h>
#include <support/ansi.h>
#include <support/bit_enum.h>
#include <support/conversions.h>
#include <support/math.h>
#include <support/type_traits.h>
#include <support/utility.h>
#include <vm/executable.h>
#include <vm/instruction.h>
#include <vm/machine.h>
#include <vm/stack.h>
#include <vm/value.h>
#include <vm/value_ref.h>
