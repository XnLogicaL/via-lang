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
namespace ansi {

// ANSI text foreground color codes.
enum class Foreground
{
    NONE,
    RED = 31,
    GREEN = 32,
    BLUE = 34,
    YELLOW = 33,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37,
    BLACK = 30
};

// ANSI text background color codes.
enum class Background
{
    NONE,
    RED = 41,
    GREEN = 42,
    BLUE = 44,
    YELLOW = 43,
    MAGENTA = 45,
    CYAN = 46,
    WHITE = 47,
    BLACK = 40
};

// ANSI text style color codes.
enum class Style
{
    NONE,
    RESET = 0,
    BOLD = 1,
    FAINT = 2,
    ITALIC = 3,
    UNDERLINE = 4,
};

std::string format(
    std::string string,
    Foreground fg,
    Background bg = Background::NONE,
    Style style = Style::NONE
);

} // namespace ansi
} // namespace via
