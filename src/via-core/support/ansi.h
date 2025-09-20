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
    Red = 31,
    Green = 32,
    Blue = 34,
    Yellow = 33,
    Magenta = 35,
    Cyan = 36,
    White = 37,
    Black = 30
};

// ANSI text background color codes.
enum class Background
{
    Red = 41,
    Green = 42,
    Blue = 44,
    Yellow = 43,
    Magenta = 45,
    Cyan = 46,
    White = 47,
    Black = 40
};

// ANSI text style color codes.
enum class Style
{
    Reset = 0,
    Bold = 1,
    Faint = 2,
    Italic = 3,
    Underline = 4,
};

std::string format(
    std::string string,
    Foreground foreground,
    Background background = Background::Black,
    Style style = Style::Reset
);

} // namespace ansi
} // namespace via
