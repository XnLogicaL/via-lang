// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_COLOR_H
#define VIA_COLOR_H

#include "common.h"

namespace via {

// ANSI text foreground color codes.
enum FGColor {
  FG_RED = 31,
  FG_GREEN = 32,
  FG_BLUE = 34,
  FG_YELLOW = 33,
  FG_MAGENTA = 35,
  FG_CYAN = 36,
  FG_WHITE = 37,
  FG_BLACK = 30
};

// ANSI text background color codes.
enum BGColor {
  BG_RED = 41,
  BG_GREEN = 42,
  BG_BLUE = 44,
  BG_YELLOW = 43,
  BG_MAGENTA = 45,
  BG_CYAN = 46,
  BG_WHITE = 47,
  BG_BLACK = 40
};

// ANSI text style color codes.
enum Style {
  ST_RESET = 0,
  ST_BOLD = 1,
  ST_FAINT = 2,
  ST_ITALIC = 3,
  ST_UNDERLINE = 4,
};

// Function to wrap a String with ANSI escape codes
inline String apply_color(String str, FGColor fg, BGColor bg = BG_BLACK, Style style = ST_RESET) {
  // Construct ANSI escape code and apply the color formatting to the text
  return "\033[" + std::to_string(static_cast<int>(style)) + ";"
    + std::to_string(static_cast<int>(fg)) + ";" + std::to_string(static_cast<int>(bg)) + "m" + str
    + "\033[0m";
}

} // namespace via

#endif
