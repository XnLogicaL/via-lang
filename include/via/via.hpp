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
    #include "config.hpp"
    #include "policy.hpp"
#endif

#include <ast/ast.hpp>
#include <init.hpp>
#include <lexer/lexer.hpp>
#include <lexer/token.hpp>
#include <logger.hpp>
#include <module/defs.hpp>
#include <module/manager.hpp>
#include <module/module.hpp>
#include <module/symbol.hpp>
#include <parser/parser.hpp>
#include <sema/const.hpp>
#include <sema/register.hpp>
#include <support/ansi.hpp>
#include <vm/executable.hpp>
#include <vm/instruction.hpp>
#include <vm/machine.hpp>
#include <vm/ref.hpp>
#include <vm/stack.hpp>
#include <vm/value.hpp>
