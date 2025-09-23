/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.h>
#include <via/types.h>

namespace via {

struct SourceLoc
{
    size_t begin;
    size_t end;
};

struct RelSourceLoc
{
    size_t line;
    size_t offset;

    RelSourceLoc(const std::string& source, SourceLoc loc);
};

} // namespace via
