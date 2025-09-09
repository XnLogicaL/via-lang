/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <ansi.h>
#include <ast/ast.h>
#include <constexpr_ipow.h>
#include <constexpr_stof.h>
#include <constexpr_stoi.h>
#include <diagnostics.h>
#include <iota.h>
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
#include <sema/constexpr.h>
#include <sema/register.h>
#include <semver.h>
#include <vm/executable.h>
#include <vm/instruction.h>
#include <vm/interpreter.h>
#include <vm/stack.h>
#include <vm/value.h>
#include <vm/value_ref.h>
#include "policy.h"

