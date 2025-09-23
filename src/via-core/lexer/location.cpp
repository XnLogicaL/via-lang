/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "location.h"

via::RelSourceLoc::RelSourceLoc(const std::string& source, SourceLoc loc)
{
    size_t init = 0;
    for (size_t i = 0; i < loc.begin; ++i) {
        if (source[i] == '\n') {
            ++line;
            init = i + 1;
        }
    }

    offset = loc.begin - init;
}
